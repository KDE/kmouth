/***************************************************************************
                          phrasebook.h  -  description
                             -------------------
    begin                : Don Sep 19 2002
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

#ifndef PHRASEBOOK_H
#define PHRASEBOOK_H

#include <qobject.h>
#include <q3dragobject.h>
#include <qxml.h>
//Added by qt3to4:
#include <QTextStream>
#include <Q3PopupMenu>

#include <kaction.h>
#include <kprinter.h>

class KURL;

/**
 * The class Phrase represents one phrase in a phrase book.
 * @author Gunnar Schmi Dt
 */
class Phrase {
   friend class PhraseBookParser;
public:
   Phrase();
   Phrase (const QString &phrase);
   Phrase (const QString &phrase, const QString &shortcut);

   QString getPhrase() const;
   QString getShortcut() const;

   void setPhrase (const QString &phrase);
   void setShortcut (const QString &shortcut);

private:
   QString phrase;
   QString shortcut;
};

/**
 * The class PhraseBookEntry implements a phrase book entry. That can be either
 * a phrase or a start tag a sub phrase book.
 * @author Gunnar Schmi Dt
 */
class PhraseBookEntry {
public:
   PhraseBookEntry ();
   PhraseBookEntry (Phrase phrase, int level = 1, bool isPhrase = true);
   ~PhraseBookEntry () {};

   void setPhrase (Phrase phrase, int level = 1, bool isPhrase = true);

   bool isPhrase() const;
   Phrase getPhrase() const;
   int getLevel() const;

private:
   bool isPhraseValue;
   Phrase phrase;
   int level;
};

typedef QList<PhraseBookEntry> PhraseBookEntryList;

/**
 * The class PhraseBook implements a phrase book. It mainly stores a
 * token list where each token is a phrase book entry (either a phrase
 * or a sub phrase book). The entries are placed into a tree structure
 * as follows:
 *
 * The level of each entry tells the level in the tree (level=0 is the top
 * level), each sub book in level i directly or indirectly contains all
 * following entries until an entry of level at most i or the end of the
 * token list.
 *
 * @author Gunnar Schmi Dt
 */
class PhraseBook : public PhraseBookEntryList {
public:
   PhraseBook() : PhraseBookEntryList() {};
   ~PhraseBook() {};

   /** opens a file containing a phrase book. Returns true if successful. */
   bool open (const KURL &url);

   /** decodes a phrase book. Returns true if successful. */
   bool decode (const QString &xml);

   /** decodes a phrase book. Returns true if successful. */
   bool decode (QXmlInputSource &source);

   /** Writes the phrases to a file. Returns true if successful. */
   bool save (const KURL &url);

   /** Writes the phrases to a file. Returns true if successful. */
   bool save (const KURL &url, bool asPhrasebook);

   /** Writes the phrases to a QTextStream. */
   void save (QTextStream &stream, bool asPhrasebook);

   /** Prints the phrases. */
   void print (KPrinter *pPrinter);

   /** Shows a file selector and writes the phrases to a file.
    *  @return 1, if the file got successfully written,
    *          0, if the user canceled the operation,
    *         -1, if there was an error when saving the file.
    */
   int save (QWidget *parent, const QString &title, KURL &url, bool phrasebookFirst = true);

   /** encodes the phrase book. Returns the encoded xml code. */
   QString encode ();

   /** Stores all entries in a QStringList. All hierarchy information and all
    * shortcuts are ignored during this operation.
    */
   QStringList toStringList();

   /** Adds the entries of the book to both the given popup menu and the given
    * toolbar. The corresponding actions will be inserted into phrases.
    */
   void addToGUI (Q3PopupMenu *popup, KToolBar *toolbar,
                  KActionCollection *phrases,
                  QObject *receiver, const char *slot) const;

   /** Inserts book into a new sub phrase book.
    * @param name the name of the new sub phrase book.
    * @param book the phrase book to insert.
    */
   void insert (const QString &name, const PhraseBook &book);
};

/**
 * The class PhraseBookDrag implements drag and drop support for phrase books.
 * @author Gunnar Schmi Dt
 */
class PhraseBookDrag: public Q3DragObject {
   Q_OBJECT
public:
   PhraseBookDrag (PhraseBook *book, QWidget *dragSource = 0, const char *name = 0);
   PhraseBookDrag (QWidget *dragSource = 0, const char *name = 0);
   ~PhraseBookDrag ();

   virtual void setBook (PhraseBook *book);

   const char *format (int i) const;
   virtual QByteArray encodedData (const char *) const;

   static bool canDecode (const QMimeSource *e);
   static bool decode (const QMimeSource *e, PhraseBook *book);

private:
   bool isEmpty;
   Q3TextDrag xmlphrasebook;
   Q3TextDrag xml;
   Q3TextDrag plain;
};

class PhraseAction : public KAction {
   Q_OBJECT
public:
   PhraseAction (const QString& phrase, const QString& cut, const QObject* receiver, const char* slot, KActionCollection* parent)
   : KAction (phrase, "phrase", KShortcut(cut), 0, 0, parent, phrase.latin1()) {
      this->phrase = phrase;
      connect (this, SIGNAL(slotActivated (const QString &)), receiver, slot);
   };
   ~PhraseAction () {
   }

public slots:
   void slotActivated () {
      KAction::slotActivated();
      emit slotActivated (phrase);
   }

signals:
   void slotActivated (const QString &phrase);

private:
   QString phrase;
};

#endif
