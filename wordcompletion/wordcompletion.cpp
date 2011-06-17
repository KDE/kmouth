#include <tqregexp.h>
#include <tqfile.h>

#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include "wordcompletion.h"

class WordCompletion::WordCompletionPrivate {
friend class WordCompletion;
private:
   typedef TQMap<TQString,int> WordMap;
   struct DictionaryDetails {
      TQString filename;
      TQString language;
   };

   TQString lastText;
   TQMap<TQString,int> map;
   TQMap<TQString,int> addedWords;
   TQMap<TQString,DictionaryDetails> dictDetails;
   TQStringList dictionaries;
   TQString current;
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

typedef TQPair<int,TQString> Match;
typedef TQValueList<Match> MatchList;

TQString WordCompletion::makeCompletion(const TQString &text) {
   if (d->lastText != text) {
      d->lastText = text;
      KCompletion::clear();

      int border = text.tqfindRev(TQRegExp("\\W"));
      TQString suffix = text.right (text.length() - border - 1).lower();
      TQString prefix = text.left (border + 1);

      if (suffix.length() > 0) {
         MatchList matches;
         TQMap<TQString,int>::ConstIterator it;
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

TQStringList WordCompletion::wordLists() {
   return d->dictionaries;
}

TQStringList WordCompletion::wordLists(const TQString &language) {
   TQStringList result;
   for (TQStringList::Iterator it = d->dictionaries.begin();
         it != d->dictionaries.end(); ++it)
      if (d->dictDetails[*it].language == language)
         result += *it;
   return result;
}

TQString WordCompletion::languageOfWordList(const TQString &wordlist) {
   if (d->dictDetails.tqcontains(wordlist))
      return d->dictDetails[wordlist].language;
   else
      return TQString();
}

TQString WordCompletion::currentWordList() {
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
   TQStringList groups = config->groupList();
   for (TQStringList::Iterator it = groups.begin(); it != groups.end(); ++it)
      if ((*it).startsWith ("Dictionary ")) {
         config->setGroup(*it);
         WordCompletionPrivate::DictionaryDetails details;
         details.filename = config->readEntry("Filename");
         details.language = config->readEntry("Language");
         TQString name = config->readEntry("Name");
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

bool WordCompletion::setWordList(const TQString &wordlist) {
   if (d->wordsToSave)
      save ();
   d->wordsToSave = false;

   d->map.clear();
   bool result = d->dictDetails.tqcontains (wordlist);
   if (result)
      d->current = wordlist;
   else
      d->current = d->dictionaries[0];
   
   TQString filename = d->dictDetails[d->current].filename;
   TQString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", filename);
   TQFile file(dictionaryFile);
   if (file.exists() && file.open(IO_ReadOnly)) {
      TQTextStream stream(&file);
      stream.setEncoding (TQTextStream::UnicodeUTF8);
      if (!stream.atEnd()) {
         if (stream.readLine() == "WPDictFile") {
            while (!stream.atEnd()) {
               TQString s = stream.readLine();
               if (!(s.isNull() || s.isEmpty())) {
                  TQStringList list = TQStringList::split("\t", s);
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

void WordCompletion::addSentence (const TQString &sentence) {
   TQStringList words = TQStringList::split(TQRegExp("\\W"), sentence);
   
   TQStringList::ConstIterator it;
   for (it = words.begin(); it != words.end(); ++it) {
      if (!(*it).tqcontains(TQRegExp("\\d|_"))) {
         TQString key = (*it).lower();
         if (d->map.tqcontains(key))
            d->map[key] += 1;
         else
            d->map[key] = 1;
         if (d->addedWords.tqcontains(key))
            d->addedWords[key] += 1;
         else
            d->addedWords[key] = 1;
      }
   }
   d->wordsToSave = true;
}

void WordCompletion::save () {
   if (d->wordsToSave) {
      TQString filename = d->dictDetails[d->current].filename;
      TQString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", filename);
      TQFile file(dictionaryFile);
      if (!file.exists())
         return;
      if (!file.open(IO_WriteOnly))
         return;

      TQTextStream stream(&file);
      stream.setEncoding (TQTextStream::UnicodeUTF8);

      stream << "WPDictFile\n";
      TQMap<TQString,int>::ConstIterator it;
      for (it = d->map.begin(); it != d->map.end(); ++it) {
         if (d->addedWords.tqcontains(it.key())) {
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

