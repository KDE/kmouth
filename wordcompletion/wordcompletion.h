#ifndef WORDCOMPLETION_H
#define WORDCOMPLETION_H

#include <kcompletion.h>

/**
 * This class does completion based on a dictionary of words.
 */
class WordCompletion : public KCompletion {
   friend class WordListWidget;
   Q_OBJECT
public:
   WordCompletion(QString dictionary);
   virtual ~WordCompletion();

   /**
    * Finds completions to the given text.
    */
   virtual QString makeCompletion(const QString&);

   bool isConfigured();
   void configure(QString dictionary, QString language, QString dicFile);

private:
   class WordCompletionPrivate;
   WordCompletionPrivate *d;
};

#endif // KURLCOMPLETION_H
