/***************************************************************************
                          wordlist.cpp  -  description
                             -------------------
    begin                : Don Sep 12 2002
    copyright            : (C) 2002 by Gunnar Schmi Dt
    email                : kmouth@schmi-dt.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// $Id$

#include <qregexp.h>
#include "wordlist.h"

#include <qapplication.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <klocale.h>

namespace WordList {

XMLParser::XMLParser() {
}

XMLParser::~XMLParser() {
}

bool XMLParser::warning (const QXmlParseException &) {
   return false;
}

bool XMLParser::error (const QXmlParseException &) {
   return false;
}

bool XMLParser::fatalError (const QXmlParseException &) {
   return false;
}

QString XMLParser::errorString() {
   return "";
}

bool XMLParser::startDocument() {
   text = QString::null;
   return true;
}

bool XMLParser::startElement (const QString &, const QString &,
                                     const QString &,
                                     const QXmlAttributes &)
{
   if (!text.isNull() && !text.isEmpty()) {
      list += QStringList::split(QRegExp("\\W"), text);
      text = QString::null;
   }
   return true;
}

bool XMLParser::characters (const QString &ch) {
   text += ch;
   return true;
}

bool XMLParser::ignorableWhitespace (const QString &) {
   if (!text.isNull() && !text.isEmpty()) {
      list += QStringList::split(QRegExp("\\W"), text);
      text = QString::null;
   }
   return true;
}

bool XMLParser::endElement (const QString &, const QString &,
                                   const QString &)
{
   if (!text.isNull() && !text.isEmpty()) {
      list += QStringList::split(QRegExp("\\W"), text);
      text = QString::null;
   }
   return true;
}

bool XMLParser::endDocument() {
   if (!text.isNull() && !text.isEmpty()) {
      list += QStringList::split(QRegExp("\\W"), text);
      text = QString::null;
   }
   return true;
}

QStringList XMLParser::getList() {
   return list;
}


/***************************************************************************/

/* Structures used for storing *.aff files (part of OpenOffice.org dictionaries)
 */
struct AffEntry {
   bool    cross;
   int     charsToRemove;
   QString add;
   QStringList condition;
};
typedef QValueList<AffEntry> AffList;
typedef QMap<QChar,AffList>  AffMap;

/** Loads an *.aff file (part of OpenOffice.org dictionaries)
 */
void loadAffFile(const QString &filename, AffMap &prefixes, AffMap &suffixes) {
   bool cross = false;

   QFile afile(filename);
   if (afile.open(IO_ReadOnly)) {
      QTextStream stream(&afile);
      while (!stream.atEnd()) {
         QString s = stream.readLine();
         QStringList fields = QStringList::split(QRegExp("\\s"), s);

         if (fields.count() == 4) {
            cross = (fields[2] == "Y");
         }
         else {
            if (fields.count() >= 5) {
               AffEntry e;
               e.cross     = cross;
               if (fields[2] == "0")
                  e.charsToRemove = 0;
               else
                  e.charsToRemove = fields[2].length();
               e.add       = fields[3];

               if (fields[4] != ".") {
                  QString condition = fields[4];
                  for (uint idx = 0; idx < condition.length(); ++idx) {
                     if (condition[idx] == '[') {
                        QString code;
                        for (++idx; (idx < condition.length()) && condition[idx] != ']'; ++idx)
                           code += condition[idx];
                        e.condition << code;
                     }
                     else
                        e.condition << QString(condition[idx]);
                  }
               }
               
               if (s.startsWith("PFX")) {
                  AffList list;
                  if (prefixes.contains (fields[1][0]))
                     list = prefixes[fields[1][0]];
                  list << e;
                  prefixes[fields[1][0]] = list;
               }
               else if (s.startsWith("SFX")) {
                  AffList list;
                  if (suffixes.contains (fields[1][0]))
                     list = suffixes[fields[1][0]];
                  list << e;
                  suffixes[fields[1][0]] = list;
               }
            }
         }
      }
   }
}

/** Checks if the given word matches the given condition. Each entry of the
 * QStringList "condition" describes one character of the word. (If the word
 * has more characters than condition entries only the last characters are
 * compared).
 * Each entry contains either all valid characters (if it does _not_ start
 * with "^") or all invalid characters (if it starts with "^").
 */
inline bool checkCondition (const QString &word, const QStringList &condition) {
   if (condition.count() == 0)
      return true;

   if (word.length() < condition.count())
      return false;
   
   QStringList::ConstIterator it;
   int idx;
   for (it = condition.begin(), idx = word.length()-condition.count();
        it != condition.end();
        ++it, ++idx)
   {
      if ((*it).contains(word[idx]) == ((*it)[0] == '^'))
         return false;
   }
   return true;
}

/** Constructs words by adding suffixes to the given word, and copies the
 * resulting words from map to checkedMap.
 * @param modifiers discribes which suffixes are valid
 * @param cross true if the word has a prefix
 */
inline void checkWord(const QString &word, const QString &modifiers, bool cross, const WordMap &map, WordMap &checkedMap, const AffMap &suffixes) {
   for (uint i = 0; i < modifiers.length(); i++) {
      if (suffixes.contains(modifiers[i])) {
         AffList sList = suffixes[modifiers[i]];

         AffList::ConstIterator sIt;
         for (sIt = sList.begin(); sIt != sList.end(); ++sIt) {
            if (((*sIt).cross || !cross)
             && (checkCondition(word, (*sIt).condition)))
            {
               QString sWord = word.left(word.length()-(*sIt).charsToRemove) + (*sIt).add;
               if (map.contains(sWord))
                  checkedMap[sWord] = map[sWord];
            }
         }
      }
   }
}

/** Constructs words by adding pre- and suffixes to the given word, and
 * copies the resulting words from map to checkedMap.
 * @param modifiers discribes which pre- and suffixes are valid
 */
void checkWord (const QString &word, const QString &modifiers, const WordMap &map, WordMap &checkedMap, const AffMap &prefixes, const AffMap &suffixes) {
   if (map.contains(word))
      checkedMap[word] = map[word];

   checkWord(word, modifiers, true, map, checkedMap, suffixes);

   for (uint i = 0; i < modifiers.length(); i++) {
      if (prefixes.contains(modifiers[i])) {
         AffList pList = prefixes[modifiers[i]];

         AffList::ConstIterator pIt;
         for (pIt = pList.begin(); pIt != pList.end(); ++pIt) {
            QString pWord = (*pIt).add + word;
            if (map.contains(pWord))
               checkedMap[pWord] = map[pWord];

            checkWord(pWord, modifiers, false, map, checkedMap, suffixes);
         }
      }
   }
}

/***************************************************************************/

void addWordsFromFile (WordMap &map, QString filename) {
   QFile file(filename);
   QXmlInputSource source (&file);
   XMLParser parser;
   QXmlSimpleReader reader;
   reader.setFeature ("http://trolltech.com/xml/features/report-start-end-entity", true);
   reader.setContentHandler (&parser);
   
   if (reader.parse(source)) {
      QStringList words = parser.getList();
      QStringList::ConstIterator it;
      for (it = words.begin(); it != words.end(); ++it) {
         if (!(*it).contains(QRegExp("\\d|_"))) {
            QString key = (*it).lower();
            if (map.contains(key))
               map[key] += 1;
            else
               map[key] = 1;
         }
      }
   }
}

WordMap checkWordList(WordMap &map, QString dict, QString aff);

WordMap createWordList (QString language, QString dictionary) {
   KProgressDialog *pdlg = new KProgressDialog(0, "progressDialog", i18n("Creating word list"), i18n("Parsing the KDE documentation..."), false);
   pdlg->setAllowCancel (false);
   pdlg->showCancelButton (false);
   pdlg->setLabel (i18n("Parsing the KDE documentation..."));
   pdlg->setAutoReset(false);
   pdlg->setAutoClose(false);
   pdlg->progressBar()->setTotalSteps(100);
   pdlg->progressBar()->setProgress(0);
   pdlg->show();
   qApp->processEvents (20);
   
   int progress = 0;
   int steps = 0;
   int percent = 0;
         
   QStringList files = KApplication::kApplication()->dirs()->findAllResources ("html", language + "/*.docbook", true, true);
   if ((files.count() == 0) && (language.length() == 5)) {
      language = language.left(2);
      files = KApplication::kApplication()->dirs()->findAllResources ("html", language + "/*.docbook", true, true);
   }

   steps = files.count();
   
   WordMap map;
   QStringList::ConstIterator it;
   for (progress = 1, it = files.begin(); it != files.end(); ++progress, ++it) {
      addWordsFromFile (map, *it);

      if (steps != 0 && progress*100/steps > percent) {
         percent = progress*100/steps;
         pdlg->progressBar()->setProgress(percent);
         qApp->processEvents (20);
      }
   }

   if (dictionary.endsWith(".dic")) {
      AffMap prefixes;
      AffMap suffixes;
      WordMap checkedMap;
      loadAffFile (dictionary.left(dictionary.length()-4) + ".aff", prefixes, suffixes);

      pdlg->progressBar()->reset();
      pdlg->setLabel (i18n("Performing spell check..."));
      pdlg->progressBar()->setTotalSteps(100);
      pdlg->progressBar()->setProgress(0);
      qApp->processEvents (20);
      progress = 0;
      steps = 0;
      percent = 0;

      QFile dfile(dictionary);
      if (dfile.open(IO_ReadOnly)) {
         QTextStream stream(&dfile);
         
         if (!stream.atEnd()) {
            QString s = stream.readLine(); // Number of words
            steps = s.toInt();
         }

         while (!stream.atEnd()) {
            QString s = stream.readLine();
            if (s.contains("/")) {
               QString word = s.left(s.find("/")).lower();
               QString modifiers = s.right(s.length() - s.find("/"));
            
               checkWord(word, modifiers, map, checkedMap, prefixes, suffixes);
            }
            else {
               if (!s.isEmpty() && !s.isNull() && map.contains(s.lower()))
                  checkedMap[s.lower()] = map[s.lower()];
            }
            
            progress++;
            if (steps != 0 && progress*100/steps > percent) {
               percent = progress*100/steps;
               pdlg->progressBar()->setProgress(percent);
               qApp->processEvents (20);
            }
         }
      }

      pdlg->close();
      delete pdlg;
      return checkedMap;
   }
   else {
      pdlg->close();
      delete pdlg;
      return map;
   }
}

}
