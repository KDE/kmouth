#include <qregexp.h>
#include <qfile.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include "wordcompletion.h"

class WordCompletion::WordCompletionPrivate {
friend class WordCompletion;
private:
   typedef QMap<QString,int> WordMap;
   struct DictionaryDetails {
      QString filename;
      QString language;
   };

   QString lastText;
   QMap<QString,int> map;
   QMap<QString,int> addedWords;
   QMap<QString,DictionaryDetails> dictDetails;
   QStringList dictionaries;
   QString current;
   bool blockCurrentListSignal;
   bool wordsToSave;
};

WordCompletion::WordCompletion() : KCompletion () {
   d = new WordCompletionPrivate();
   d->blockCurrentListSignal = false;
   d->wordsToSave = false;
   configure ();
}

WordCompletion::~WordCompletion() {
   save ();
   delete d;
}

typedef QPair<int,QString> Match;
typedef QValueList<Match> MatchList;

QString WordCompletion::makeCompletion(const QString &text) {
   if (d->lastText != text) {
      d->lastText = text;
      KCompletion::clear();

      int border = text.findRev(QRegExp("\\W"));
      QString suffix = text.right (text.length() - border - 1).lower();
      QString prefix = text.left (border + 1);

      if (suffix.length() > 0) {
         MatchList matches;
         QMap<QString,int>::ConstIterator it;
         for (it = d->map.begin(); it != d->map.end(); ++it)
            if (it.key().startsWith(suffix))
               matches += Match (-it.data(), it.key());
         qHeapSort(matches);

         MatchList::ConstIterator iter = matches.begin();
         for (int count = 0; (iter != matches.end()) && (count < 10); ++iter, ++count) {
            int length = (*iter).second.length() + prefix.length() - text.length();
            KCompletion::addItem(text + (*iter).second.right(length), -(*iter).first);
         }
      }
   }

   // call the KCompletion::makeCompletion(...) method
   return KCompletion::makeCompletion (text);
}

QStringList WordCompletion::wordLists() {
   return d->dictionaries;
}

QStringList WordCompletion::wordLists(const QString &language) {
   QStringList result;
   for (QStringList::Iterator it = d->dictionaries.begin();
         it != d->dictionaries.end(); ++it)
      if (d->dictDetails[*it].language == language)
         result += *it;
   return result;
}

QString WordCompletion::languageOfWordList(const QString &wordlist) {
   if (d->dictDetails.contains(wordlist))
      return d->dictDetails[wordlist].language;
   else
      return QString::null;
}

QString WordCompletion::currentWordList() {
   return d->current;
}

bool WordCompletion::isConfigured() {
   KConfig *config = new KConfig("kmouthrc");
   bool result = config->hasGroup("Dictionary 0");
   delete config;

   return result;
}

void WordCompletion::configure() {
   if (d->wordsToSave)
      save ();
   d->wordsToSave = false;

   d->dictionaries.clear();
   d->dictDetails.clear();

   KConfig *config = new KConfig("kmouthrc");
   QStringList groups = config->groupList();
   for (QStringList::Iterator it = groups.begin(); it != groups.end(); ++it)
      if ((*it).startsWith ("Dictionary ")) {
         config->setGroup(*it);
         WordCompletionPrivate::DictionaryDetails details;
         details.filename = config->readEntry("Filename");
         details.language = config->readEntry("Language");
         QString name = config->readEntry("Name");
         d->dictDetails[name] = details;
         d->dictionaries += name;
      }
   delete config;
   
   d->blockCurrentListSignal = true;
   setWordList(d->current);
   d->blockCurrentListSignal = false;
   emit wordListsChanged (wordLists());
   emit currentListChanged (d->current);
}

bool WordCompletion::setWordList(const QString &wordlist) {
   if (d->wordsToSave)
      save ();
   d->wordsToSave = false;

   d->map.clear();
   bool result = d->dictDetails.contains (wordlist);
   if (result)
      d->current = wordlist;
   else
      d->current = d->dictionaries[0];
   
   QString filename = d->dictDetails[d->current].filename;
   QString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", filename);
   QFile file(dictionaryFile);
   if (file.exists() && file.open(IO_ReadOnly)) {
      QTextStream stream(&file);
      stream.setEncoding (QTextStream::UnicodeUTF8);
      if (!stream.atEnd()) {
         if (stream.readLine() == "WPDictFile") {
            while (!stream.atEnd()) {
               QString s = stream.readLine();
               if (!(s.isNull() || s.isEmpty())) {
                  QStringList list = QStringList::split("\t", s);
                  bool ok;
                  int weight = list[1].toInt(&ok);
                  if (ok && (weight > 0))
                     d->map [list[0]] = weight;
               }
            }
         }
      }
      file.close();
   }
   if (!d->blockCurrentListSignal)
      emit currentListChanged (d->current);
   d->lastText = "";
   d->wordsToSave = false;
   return result;
}

void WordCompletion::addSentence (const QString &sentence) {
   QStringList words = QStringList::split(QRegExp("\\W"), sentence);
   
   QStringList::ConstIterator it;
   for (it = words.begin(); it != words.end(); ++it) {
      if (!(*it).contains(QRegExp("\\d|_"))) {
         QString key = (*it).lower();
         if (d->map.contains(key))
            d->map[key] += 1;
         else
            d->map[key] = 1;
         if (d->addedWords.contains(key))
            d->addedWords[key] += 1;
         else
            d->addedWords[key] = 1;
      }
   }
   d->wordsToSave = true;
}

void WordCompletion::save () {
   if (d->wordsToSave) {
      QString filename = d->dictDetails[d->current].filename;
      QString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", filename);
      QFile file(dictionaryFile);
      if (!file.exists())
         return;
      if (!file.open(IO_WriteOnly))
         return;

      QTextStream stream(&file);
      stream.setEncoding (QTextStream::UnicodeUTF8);

      stream << "WPDictFile\n";
      QMap<QString,int>::ConstIterator it;
      for (it = d->map.begin(); it != d->map.end(); ++it) {
         if (d->addedWords.contains(it.key())) {
            stream << it.key() << "\t" << d->addedWords[it.key()] << "\t1\n";
            stream << it.key() << "\t" << it.data() - d->addedWords[it.key()] << "\t2\n";
         }
         else
            stream << it.key() << "\t" << it.data() << "\t2\n";
      }
      file.close();
      d->wordsToSave = false;
   }
}

#include "wordcompletion.moc"

