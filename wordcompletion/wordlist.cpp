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

#include <tqregexp.h>
#include <tqdir.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <klocale.h>

#include "wordlist.h"

namespace WordList {
void addWords (WordMap &map, TQString line);

XMLParser::XMLParser() {
}

XMLParser::~XMLParser() {
}

bool XMLParser::warning (const TQXmlParseException &) {
   return false;
}

bool XMLParser::error (const TQXmlParseException &) {
   return false;
}

bool XMLParser::fatalError (const TQXmlParseException &) {
   return false;
}

TQString XMLParser::errorString() {
   return "";
}

bool XMLParser::startDocument() {
   text = TQString::null;
   return true;
}

bool XMLParser::startElement (const TQString &, const TQString &,
                                     const TQString &,
                                     const TQXmlAttributes &)
{
   if (!text.isNull() && !text.isEmpty()) {
      addWords (list, text);
      text = TQString::null;
   }
   return true;
}

bool XMLParser::characters (const TQString &ch) {
   text += ch;
   return true;
}

bool XMLParser::ignorableWhitespace (const TQString &) {
   if (!text.isNull() && !text.isEmpty()) {
      addWords (list, text);
      text = TQString::null;
   }
   return true;
}

bool XMLParser::endElement (const TQString &, const TQString &,
                                   const TQString &)
{
   if (!text.isNull() && !text.isEmpty()) {
      addWords (list, text);
      text = TQString::null;
   }
   return true;
}

bool XMLParser::endDocument() {
   if (!text.isNull() && !text.isEmpty()) {
      addWords (list, text);
      text = TQString::null;
   }
   return true;
}

WordMap XMLParser::getList() {
   return list;
}

/***************************************************************************/

KProgressDialog *progressDialog() {
   KProgressDialog *pdlg = new KProgressDialog(0, "progressDialog", i18n("Creating Word List"), i18n("Parsing the KDE documentation..."), false);
   pdlg->setAllowCancel (false);
   pdlg->showCancelButton (false);
   pdlg->setAutoReset(false);
   pdlg->setAutoClose(false);
   pdlg->progressBar()->setTotalSteps(100);
   pdlg->progressBar()->setProgress(0);
   return pdlg;
}

bool saveWordList (WordMap map, TQString filename) {
   TQFile file(filename);
   if(!file.open(IO_WriteOnly))
      return false;

   TQTextStream stream(&file);
   stream.setEncoding (TQTextStream::UnicodeUTF8);

   stream << "WPDictFile\n";
   WordMap::ConstIterator it;
   for (it = map.begin(); it != map.end(); ++it)
      stream << it.key() << "\t" << it.data() << "\t2\n";
   file.close();
   return true;
}

/***************************************************************************/

void addWords (WordMap &map, TQString line) {
   TQStringList words = TQStringList::split(TQRegExp("\\W"), line);
   
   TQStringList::ConstIterator it;
   for (it = words.begin(); it != words.end(); ++it) {
      if (!(*it).contains(TQRegExp("\\d|_"))) {
         TQString key = (*it).lower();
         if (map.contains(key))
            map[key] += 1;
         else
            map[key] = 1;
      }
   }
}

void addWords (WordMap &map, WordMap add) {
   WordList::WordMap::ConstIterator it;
   for (it = add.begin(); it != add.end(); ++it)
      if (map.contains(it.key()))
         map[it.key()] += it.data();
      else
         map[it.key()] = it.data();
}

void addWordsFromFile (WordMap &map, TQString filename, TQTextStream::Encoding encoding, TQTextCodec *codec) {
   TQFile xmlfile(filename);
   TQXmlInputSource source (&xmlfile);
   XMLParser parser;
   TQXmlSimpleReader reader;
   reader.setFeature ("http://trolltech.com/xml/features/report-start-end-entity", true);
   reader.setContentHandler (&parser);
   
   WordMap words;
   if (reader.parse(source)) // try to load the file as an xml-file
      addWords(map, parser.getList());
   else {
      TQFile wpdfile(filename);
      if (wpdfile.open(IO_ReadOnly)) {
         TQTextStream stream(&wpdfile);
         stream.setEncoding (TQTextStream::UnicodeUTF8);

         if (!stream.atEnd()) {
            TQString s = stream.readLine();
            if (s == "WPDictFile") { // Contains the file a weighted word list?
               // We can assume that weighted word lists are always UTF8 coded.
               while (!stream.atEnd()) {
                  TQString s = stream.readLine();
                  if (!(s.isNull() || s.isEmpty())) {
                     TQStringList list = TQStringList::split("\t", s);
                     bool ok;
                     int weight = list[1].toInt(&ok);
                     if (ok && (weight > 0)) {
                        if (map.contains(list[0]))
                           map[list[0]] += weight;
                        else
                           map[list[0]] = weight;
                     }
                  }
               }
            }
            else { // Count the words in an ordinary text file
               TQFile file(filename);
               if (file.open(IO_ReadOnly)) {
                  TQTextStream stream(&file);
                  if (codec != 0)
                     stream.setCodec (codec);
                  else
                     stream.setEncoding (encoding);
                  while (!stream.atEnd())
                     addWords (map, stream.readLine());
               }
            }
         }
      }
   }
}

}
#include <kdebug.h>
namespace WordList {

WordMap parseFiles (TQStringList files, TQTextStream::Encoding encoding, TQTextCodec *codec, KProgressDialog *pdlg) {
   int progress = 0;
   int steps = files.count();
   int percent = 0;
   
   WordMap map;
   TQStringList::ConstIterator it;
   for (progress = 1, it = files.begin(); it != files.end(); ++progress, ++it) {
      addWordsFromFile (map, *it, encoding, codec);

      if (steps != 0 && progress*100/steps > percent) {
         percent = progress*100/steps;
         pdlg->progressBar()->setProgress(percent);
         qApp->processEvents (20);
      }
   }
   return map;
}

WordMap mergeFiles  (TQMap<TQString,int> files, KProgressDialog *pdlg) {
   pdlg->setLabel (i18n("Merging dictionaries..."));
   pdlg->show();
   qApp->processEvents (20);
   
   int progress = 0;
   int steps = files.count();
   int percent = 0;
   float totalWeight = 0;
   long long maxWeight = 0;
   
   TQMap<TQString,float> map;
   TQMap<TQString,int>::ConstIterator it;
   for (progress = 1, it = files.begin(); it != files.end(); ++progress, ++it) {
      WordMap fileMap;
      addWordsFromFile (fileMap, it.key(), TQTextStream::UnicodeUTF8, 0);

      long long weight = 0;
      WordMap::ConstIterator iter;
      for (iter = fileMap.begin(); iter != fileMap.end(); ++iter)
         weight += iter.data();
      float factor = 1.0 * it.data() / weight;
      totalWeight += it.data();
      if (weight > maxWeight)
         maxWeight = weight;
      
      for (iter = fileMap.begin(); iter != fileMap.end(); ++iter)
         if (map.contains(iter.key()))
            map[iter.key()] += iter.data() * factor;
         else
            map[iter.key()] = iter.data() * factor;

      if (steps != 0 && progress*100/steps > percent) {
         percent = progress*100/steps;
         pdlg->progressBar()->setProgress(percent);
         qApp->processEvents (20);
      }
   }
   
   float factor;
   if (1.0 * maxWeight * totalWeight > 1000000000)
      factor = 1000000000 / totalWeight;
   else
      factor = 1.0 * maxWeight;
   
   WordMap resultMap;
   TQMap<TQString,float>::ConstIterator iter;
   for (iter = map.begin(); iter != map.end(); ++iter)
      resultMap[iter.key()] = (int)(factor * iter.data() + 0.5);
   
   return resultMap;
}

WordMap parseKDEDoc (TQString language, KProgressDialog *pdlg) {
   pdlg->setLabel (i18n("Parsing the KDE documentation..."));
   pdlg->show();
   qApp->processEvents (20);
   
   TQStringList files = KApplication::kApplication()->dirs()->findAllResources ("html", language + "/*.docbook", true, true);
   if ((files.count() == 0) && (language.length() == 5)) {
      language = language.left(2);
      files = KApplication::kApplication()->dirs()->findAllResources ("html", language + "/*.docbook", true, true);
   }

   return parseFiles (files, TQTextStream::UnicodeUTF8, 0, pdlg);
}

WordMap parseFile (TQString filename, TQTextStream::Encoding encoding, TQTextCodec *codec, KProgressDialog *pdlg) {
   pdlg->setLabel (i18n("Parsing file..."));
   pdlg->show();
   qApp->processEvents (20);
   
   TQStringList files = filename;

   return parseFiles (files, encoding, codec, pdlg);
}

WordMap parseDir (TQString directory, TQTextStream::Encoding encoding, TQTextCodec *codec, KProgressDialog *pdlg) {
   pdlg->setLabel (i18n("Parsing directory..."));
   pdlg->show();
   qApp->processEvents (20);
   
   TQStringList directories;
   directories += directory;
   TQStringList files;
   for (TQStringList::Iterator it = directories.begin(); it != directories.end(); it = directories.remove(it)) {
      TQDir dir(*it);
      const QFileInfoList *entries = dir.entryInfoList ("*", TQDir::Dirs | TQDir::Files | TQDir::NoSymLinks | TQDir::Readable);
      if (entries != 0) {
         QFileInfoListIterator iter (*entries);
         while ((iter.current()) != 0) {
            TQString name = iter.current()->fileName();
            if (name != "." && name != "..") {
               if (iter.current()->isDir())
                  directories += iter.current()->filePath ();
               else
                  files += iter.current()->filePath ();
            }
            ++iter;
         }
      }
   }

   return parseFiles (files, encoding, codec, pdlg);
}

/***************************************************************************/

/* Structures used for storing *.aff files (part of OpenOffice.org dictionaries)
 */
struct AffEntry {
   bool    cross;
   int     charsToRemove;
   TQString add;
   TQStringList condition;
};
typedef TQValueList<AffEntry> AffList;
typedef TQMap<TQChar,AffList>  AffMap;

/** Loads an *.aff file (part of OpenOffice.org dictionaries)
 */
void loadAffFile(const TQString &filename, AffMap &prefixes, AffMap &suffixes) {
   bool cross = false;

   TQFile afile(filename);
   if (afile.open(IO_ReadOnly)) {
      TQTextStream stream(&afile);
      while (!stream.atEnd()) {
         TQString s = stream.readLine();
         TQStringList fields = TQStringList::split(TQRegExp("\\s"), s);

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
                  TQString condition = fields[4];
                  for (uint idx = 0; idx < condition.length(); ++idx) {
                     if (condition[idx] == '[') {
                        TQString code;
                        for (++idx; (idx < condition.length()) && condition[idx] != ']'; ++idx)
                           code += condition[idx];
                        e.condition << code;
                     }
                     else
                        e.condition << TQString(condition[idx]);
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
 * TQStringList "condition" describes one character of the word. (If the word
 * has more characters than condition entries only the last characters are
 * compared).
 * Each entry contains either all valid characters (if it does _not_ start
 * with "^") or all invalid characters (if it starts with "^").
 */
inline bool checkCondition (const TQString &word, const TQStringList &condition) {
   if (condition.count() == 0)
      return true;

   if (word.length() < condition.count())
      return false;
   
   TQStringList::ConstIterator it;
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
inline void checkWord(const TQString &word, const TQString &modifiers, bool cross, const WordMap &map, WordMap &checkedMap, const AffMap &suffixes) {
   for (uint i = 0; i < modifiers.length(); i++) {
      if (suffixes.contains(modifiers[i])) {
         AffList sList = suffixes[modifiers[i]];

         AffList::ConstIterator sIt;
         for (sIt = sList.begin(); sIt != sList.end(); ++sIt) {
            if (((*sIt).cross || !cross)
             && (checkCondition(word, (*sIt).condition)))
            {
               TQString sWord = word.left(word.length()-(*sIt).charsToRemove) + (*sIt).add;
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
void checkWord (const TQString &word, const TQString &modifiers, const WordMap &map, WordMap &checkedMap, const AffMap &prefixes, const AffMap &suffixes) {
   if (map.contains(word))
      checkedMap[word] = map[word];

   checkWord(word, modifiers, true, map, checkedMap, suffixes);

   for (uint i = 0; i < modifiers.length(); i++) {
      if (prefixes.contains(modifiers[i])) {
         AffList pList = prefixes[modifiers[i]];

         AffList::ConstIterator pIt;
         for (pIt = pList.begin(); pIt != pList.end(); ++pIt) {
            TQString pWord = (*pIt).add + word;
            if (map.contains(pWord))
               checkedMap[pWord] = map[pWord];

            checkWord(pWord, modifiers, false, map, checkedMap, suffixes);
         }
      }
   }
}

WordMap spellCheck  (WordMap map, TQString dictionary, KProgressDialog *pdlg) {

   if (dictionary.endsWith(".dic")) {
      AffMap prefixes;
      AffMap suffixes;
      WordMap checkedMap;
      loadAffFile (dictionary.left(dictionary.length()-4) + ".aff", prefixes, suffixes);

      pdlg->progressBar()->reset();
      pdlg->setAllowCancel (false);
      pdlg->showCancelButton (false);
      pdlg->setAutoReset(false);
      pdlg->setAutoClose(false);
      pdlg->setLabel (i18n("Performing spell check..."));
      pdlg->progressBar()->setTotalSteps(100);
      pdlg->progressBar()->setProgress(0);
      qApp->processEvents (20);
      int progress = 0;
      int steps = 0;
      int percent = 0;

      TQFile dfile(dictionary);
      if (dfile.open(IO_ReadOnly)) {
         TQTextStream stream(&dfile);
         
         if (!stream.atEnd()) {
            TQString s = stream.readLine(); // Number of words
            steps = s.toInt();
         }

         while (!stream.atEnd()) {
            TQString s = stream.readLine();
            if (s.contains("/")) {
               TQString word = s.left(s.find("/")).lower();
               TQString modifiers = s.right(s.length() - s.find("/"));
            
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

      return checkedMap;
   }
   else
      return map;
}

}

