#include <qregexp.h>
#include <qfile.h>

#include <kapplication.h>
#include <kstandarddirs.h>

#include "wordcompletion.h"
#include "wordlist.h"

#include <iostream>

class WordCompletion::WordCompletionPrivate {
friend class WordCompletion;
private:
   QString lastText;
   WordList::WordMap map;
};

WordCompletion::WordCompletion(QString dictionary) : KCompletion () {
   d = new WordCompletionPrivate();

   configure (dictionary, QString::null, QString::null);
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
         WordList::WordMap::ConstIterator it;
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

bool WordCompletion::isConfigured() {
   return d->map.count() > 0;
}

void WordCompletion::configure(QString dictionary, QString language, QString dicFile) {
   std::cout << dictionary.latin1() << std::endl;
   QString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", dictionary);
   std::cout << (dictionaryFile + " ").latin1() << std::endl;
   QFile file(dictionaryFile);
   if (file.open(IO_ReadOnly)) {
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
   
   if (!isConfigured() && !language.isNull() && !language.isEmpty()) {
      d->map = WordList::createWordList (language, dicFile);

      dictionaryFile = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/") + dictionary;;
      file.setName(dictionaryFile);
      if(!file.open(IO_WriteOnly))
         return;

      QTextStream stream(&file);
      stream.setEncoding (QTextStream::UnicodeUTF8);

      stream << "WPDictFile\n";
      WordList::WordMap::ConstIterator it;
      for (it = d->map.begin(); it != d->map.end(); ++it)
         stream << it.key() << "\t" << it.data() << "\t2\n";
      file.close();
   }
}

#include "wordcompletion.moc"

