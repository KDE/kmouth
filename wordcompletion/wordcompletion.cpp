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
   QMap<QString,DictionaryDetails> dictDetails;
   QStringList dictionaries;
   QString current;
   bool blockCurrentListSignal;
};

WordCompletion::WordCompletion() : KCompletion () {
   d = new WordCompletionPrivate();
   d->blockCurrentListSignal = false;
   configure ();
}

WordCompletion::~WordCompletion() {
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
   return result;
}

#include "wordcompletion.moc"

