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

// $Id$

#ifndef PHRASEBOOK_H
#define PHRASEBOOK_H

#include <qobject.h>
#include <qdragobject.h>
#include <qxml.h>

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
   Phrase (QString phrase);
   Phrase (QString phrase, QString shortcut);
   
   QString getPhrase() const;
   QString getShortcut() const;

   void setPhrase (QString &phrase);
   void setShortcut (QString &shortcut);

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

typedef QValueList<PhraseBookEntry> PhraseBookEntryList;

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

   /** opens a file containing a phrase book. Returns true if successfull. */
   bool open (const KURL &url);

   /** decodes a phrase book. Returns true if successfull. */
   bool decode (QString xml);

   /** decodes a phrase book. Returns true if successfull. */
   bool decode (QXmlInputSource &source);

   /** Writes the phrases to a file. Returns true if successfull. */
   bool save (const KURL &url);

   /** Writes the phrases to a file. Returns true if successfull. */
   bool save (const KURL &url, bool asPhrasebook);

   /** Prints the phrases. */
   void print (KPrinter *pPrinter);

   /** Shows a file selector and writes the phrases to a file.
    *  @return 1, if the file got successfully written,
    *          0, if the user canceled the operation,
    *         -1, if there was an error when saving the file.
    */
   int save (QWidget *parent, QString title, KURL &url, bool phrasebookFirst = true);

   /** encodes the phrase book. Returns the encoded xml code. */
   QString encode ();

   /** Stores all entries in a QStringList. All hierarchy information and all
    * shortcuts are ignored during this operation.
    */
   QStringList toStringList();

   /** Adds the entries of the book to both the given popup menu and the given
    * toolbar. The corresponding actions will be inserted into phrases.
    */
   void addToGUI (QPopupMenu *popup, KToolBar *toolbar,
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
class PhraseBookDrag: public QDragObject {
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
   QTextDrag xmlphrasebook;
   QTextDrag xml;
   QTextDrag plain;
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

/*
 * $Log$
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.5  2003/01/12 20:26:02  gunnar
 * Printing phrase book added
 *
 * Revision 1.4  2003/01/12 11:37:05  gunnar
 * Improved format list of file selectors / several small changes
 *
 * Revision 1.3  2002/12/06 08:55:05  gunnar
 * Improved the algorithm for creating the initial phrasebook
 *
 * Revision 1.2  2002/11/19 17:48:14  gunnar
 * Prevented both the parallel start of multiple KMouth instances and the parallel opening of multiple Phrase book edit windows
 *
 * Revision 1.1  2002/11/11 21:25:44  gunnar
 * Moved the parts concerning phrase books into a static library
 *
 * Revision 1.6  2002/10/29 16:16:05  gunnar
 * Connection from the phrase book to the phrase edit field added
 *
 * Revision 1.5  2002/10/28 16:58:34  gunnar
 * Import and export of phrase books implemented
 *
 * Revision 1.4  2002/10/24 19:36:29  gunnar
 * Drag and drop support in the phrase book edit dialog improved
 *
 * Revision 1.3  2002/10/23 22:19:30  gunnar
 * Cut, copy and paste features of the phrase book edit dialog improved
 *
 * Revision 1.2  2002/10/22 20:10:29  gunnar
 * Cut and copy of phrase book entries implemented
 *
 * Revision 1.1  2002/10/21 16:13:30  gunnar
 * phrase book format implemented
 *
 */
