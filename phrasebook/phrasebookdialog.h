/***************************************************************************
                          phrasebookdialog.h  -  description
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

#ifndef PHRASEBOOKDIALOG_H
#define PHRASEBOOKDIALOG_H

#include <qbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qlabel.h>
#include <qlistview.h> 
#include <kmainwindow.h>
#include <klineedit.h>
#include <kkeybutton.h>
#include <kurl.h>
#include "phrasebook.h"
#include "buttonboxui.h"

class QListViewItem;
class PhraseTreeItem;
class PhraseTree;
class QStringList;
class QString;
class KListView;

struct StandardBook {
   QString name;
   QString path;
   QString filename;
};
typedef QValueList<StandardBook> StandardBookList;

/**The class PhraseTreeItem is an ListViewItem for either a phrase or a phrase book.
  *@author Gunnar Schmi Dt
  */

class CheckBookItem : public QCheckListItem {
public:
   CheckBookItem (QListViewItem *parent, QListViewItem *last,
            const QString &text, const QString &name, const QString &filename);
   CheckBookItem (QListView *parent, QListViewItem *last,
            const QString &text, const QString &name, const QString &filename);
   ~CheckBookItem();

protected:
   virtual void activate ();
   virtual void stateChange (bool);
   virtual void propagateStateChange ();
   virtual void childChange (int numberDiff, int selDiff);

private:
   int numberOfBooks;
   int selectedBooks;
};

/**
 * This class represents a widget for configuring the initial phrasebook.
 * @author Gunnar Schmi Dt
 */

class InitialPhraseBookWidget : public QWidget {
   Q_OBJECT
public:
   InitialPhraseBookWidget(QWidget *parent, const char *name);
   ~InitialPhraseBookWidget();

   void createBook();

private:
   /** initializes the list of standard phrase books */
   void initStandardPhraseBooks ();

   KListView *books;
};

/**
 * The class StandardPhraseBookInsertAction implements an Action for
 * inserting a standard phrase book.
 * @author Gunnar Schmi Dt
 */
class StandardPhraseBookInsertAction : public KAction {
   Q_OBJECT
public:
   StandardPhraseBookInsertAction (const KURL &url, const QString &path, const QString& name, const QObject* receiver, const char* slot, KActionCollection* parent)
   : KAction (name, "phrasebook", 0, 0, 0, parent, 0) {
      this->url = url;
      this->path = path;
      connect (this, SIGNAL(slotActivated (const KURL &, const QString &)), receiver, slot);
   };
   ~StandardPhraseBookInsertAction () {
   };

public slots:
   void slotActivated () {
      KAction::slotActivated();
      emit slotActivated (url, path);
   };

signals:
   void slotActivated (const KURL &url, const QString &path);

private:
   KURL url;
   QString path;
};

/**
 * This class represents a widget holding the buttons of the phrase book
 * edit window.
 * @author Gunnar Schmi Dt
 */
class ButtonBoxWidget : public ButtonBoxUI {
public:
   ButtonBoxWidget (QWidget *parent = 0, const char *name = 0);
   ~ButtonBoxWidget ();

   KKeyButton *keyButton;
   QButtonGroup *group;

protected:
   QGridLayout *keyButtonPlaceLayout;
};

/**
 * The class PhraseBookDialog implements a dialog for editing phrase books.
 * @author Gunnar Schmi Dt
 */

class PhraseBookDialog : public KMainWindow {
   friend class InitialPhraseBookWidget;
   Q_OBJECT
private:
   /** Constructor. It is private because this class implements the singleton
    * pattern. For creating the instance of the dialog, use the get() method.
    */
   PhraseBookDialog ();

public:
   /** Returns a pointer to the instance of this dialog. As a part off the
    * singleton pattern it will make sure that there is at most one instance
    * of the dialog at a given time.
    */
   static PhraseBookDialog *get();

   /** Destructor. */
   ~PhraseBookDialog();

   bool queryClose ();

public slots:
   void slotTextChanged (const QString &s);
   void slotNoKey();
   void slotCustomKey();
   void capturedShortcut (const KShortcut& cut);
   
   void currentChanged (QListViewItem *item);
   void contextMenuRequested(QListViewItem *, const QPoint &pos, int);

   void slotRemove ();
   void slotCut ();
   void slotCopy ();
   void slotPaste ();

   void slotAddPhrasebook ();
   void slotAddPhrase ();

   void slotSave ();
   void slotImportPhrasebook ();
   void slotImportPhrasebook (const KURL &url, const QString &path);
   void slotExportPhrasebook ();
   void slotPrint ();

   void slotDropped (QDropEvent *e, QListViewItem *parent, QListViewItem *after);
   void slotMoved (QListViewItem *item, QListViewItem *, QListViewItem *);

signals:
   void phrasebookConfirmed (PhraseBook &book);

private:
   static StandardBookList standardPhraseBooks ();

   void initGUI();
   /** initializes the KActions of the window */
   void initActions();
   /** initializes the list of standard phrase books */
   void initStandardPhraseBooks ();
   
   QListViewItem *addBook (QListViewItem *parent, QListViewItem *after, PhraseBook *book);
   QListViewItem *addBook (QListViewItem *item, PhraseBook *book);
   
   void setShortcut (const KShortcut &cut);

   void _warning (const KKeySequence &cut, QString sAction, QString sTitle);
   
   bool isGlobalKeyPresent (const KShortcut& cut, bool warnUser);
   bool isPhraseKeyPresent (const KShortcut& cut, bool warnUser);
   bool isKeyPresent (const KShortcut& cut, bool warnUser);

   PhraseBook book;
   bool phrasebookChanged;

   PhraseTree *treeView;
   ButtonBoxWidget *buttonBox;

   KAction* fileNewPhrase;
   KAction* fileNewBook;
   KAction* fileSave;
   KAction* fileImport;
   KToolBarPopupAction* toolbarImport;
   KActionMenu* fileImportStandardBook;
   KAction* fileExport;
   KAction* filePrint;
   KAction* fileClose;
   KAction* editCut;
   KAction* editCopy;
   KAction* editPaste;
   KAction* editDelete;
};

#endif

/*
 * $Log$
 * Revision 1.17  2003/01/12 20:26:02  gunnar
 * Printing phrase book added
 *
 * Revision 1.16  2003/01/12 11:37:05  gunnar
 * Improved format list of file selectors / several small changes
 *
 * Revision 1.15  2003/01/08 16:58:40  gunnar
 * Selection of multiple phrase book entres in the editor added
 *
 * Revision 1.14  2002/12/30 11:45:29  gunnar
 * Wizard page for initial phrase book improved
 *
 * Revision 1.13  2002/12/29 12:10:38  gunnar
 * Wizard page for initial phrase book improved
 *
 * Revision 1.12  2002/12/06 08:55:05  gunnar
 * Improved the algorithm for creating the initial phrasebook
 *
 * Revision 1.11  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.10  2002/11/22 14:51:39  gunnar
 * Wizard for first start extended
 *
 * Revision 1.9  2002/11/22 09:29:12  gunnar
 * Small changes
 *
 * Revision 1.8  2002/11/22 08:48:34  gunnar
 * Implemented functionality that belongs to the new options in the options dialog
 *
 * Revision 1.7  2002/11/20 10:55:44  gunnar
 * Improved the keyboard accessibility
 *
 * Revision 1.6  2002/11/19 19:45:06  gunnar
 * Added "Do you want to save?"-question to the phrase book edit window
 *
 * Revision 1.5  2002/11/19 19:11:28  gunnar
 * Extended the tool bar of the phrase book edit window
 *
 * Revision 1.4  2002/11/19 17:48:14  gunnar
 * Prevented both the parallel start of multiple KMouth instances and the parallel opening of multiple Phrase book edit windows
 *
 * Revision 1.3  2002/11/17 17:13:18  gunnar
 * Several small changes in the phrase book edit window
 *
 * Revision 1.2  2002/11/14 21:19:57  gunnar
 * Extended the menu bar of the phrase book dialog
 *
 * Revision 1.1  2002/11/11 21:25:44  gunnar
 * Moved the parts concerning phrase books into a static library
 *
 * Revision 1.10  2002/11/06 19:15:08  gunnar
 * import of standard phrase books added
 *
 * Revision 1.9  2002/11/06 17:28:02  gunnar
 * Code for preventing double keyboard shortcuts extended for imported/dragged/pasted phrasebooks
 *
 * Revision 1.8  2002/11/04 21:13:08  gunnar
 * added code to prevent double shortcuts
 *
 * Revision 1.7  2002/10/29 16:16:06  gunnar
 * Connection from the phrase book to the phrase edit field added
 *
 * Revision 1.6  2002/10/24 19:36:29  gunnar
 * Drag and drop support in the phrase book edit dialog improved
 *
 * Revision 1.5  2002/10/23 22:19:30  gunnar
 * Cut, copy and paste features of the phrase book edit dialog improved
 *
 * Revision 1.4  2002/10/23 17:42:53  gunnar
 * Icons added to the items of the phrase book edit dialog
 *
 * Revision 1.3  2002/10/22 20:10:29  gunnar
 * Cut and copy of phrase book entries implemented
 *
 * Revision 1.2  2002/10/22 16:13:24  gunnar
 * Popup menu in the phrase book dialog added
 *
 * Revision 1.1  2002/10/21 18:30:50  gunnar
 * First version of the phrase book edit dialog added
 *
 */
