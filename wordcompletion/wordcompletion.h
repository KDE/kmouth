#ifndef WORDCOMPLETION_H
#define WORDCOMPLETION_H

#include <kcompletion.h>

/**
 * This class does completion based on a dictionary of words.
 */
class WordCompletion : public KCompletion {
   friend class WordListWidget;
   Q_OBJECT
  TQ_OBJECT
public:
   WordCompletion();
   virtual ~WordCompletion();

   /**
    * Returns the names for the available word lists
    */
   TQStringList wordLists();

   /**
    * Returns the names for those word lists that contain
    * words of a given language.
    */
   TQStringList wordLists(const TQString &language);
   
   /**
    * Returns the language of a given word list.
    */
   TQString languageOfWordList(const TQString &wordlist);

   /**
    * Returns the name of the currently active word list.
    */
   TQString currentWordList();

   /**
    * Finds completions to the given text.
    */
   virtual TQString makeCompletion(const TQString&);

   static bool isConfigured();
   
   /**
    * Adds the words from the given sentence to the list of words.
    */
   void addSentence (const TQString &sentence);
   
public slots:
   /**
    * Re-reads the configuration.
    */
   void configure();
   
   /**
    * Specify which word list gets used for the actual word completion.
    * If there is no word list with the given name the first configured
    * list gets used.
    * The method returns true if the specified word list was found.
    */
   bool setWordList(const TQString &wordlist);

   /**
    * Saves the added words to disk.
    */
   void save ();

signals:
   void wordListsChanged (const TQStringList &wordLists);
   void currentListChanged (const TQString &wordList);

private:
   class WordCompletionPrivate;
   WordCompletionPrivate *d;
};

#endif // KURLCOMPLETION_H
