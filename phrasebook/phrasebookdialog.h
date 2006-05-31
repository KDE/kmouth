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

#ifndef PHRASEBOOKDIALOG_H
#define PHRASEBOOKDIALOG_H

#include <q3button.h>
#include <QRadioButton>
#include <q3buttongroup.h>
#include <QLabel>
#include <q3listview.h> 
//Added by qt3to4:
#include <QList>
#include <QGridLayout>
#include <QDropEvent>
#include <kmainwindow.h>
#include <klineedit.h>
#include <kkeybutton.h>
#include <kurl.h>
#include "phrasebook.h"
#include "ui_buttonboxui.h"

class Q3ListViewItem;
class PhraseTreeItem;
class PhraseTree;
class QStringList;
class QString;
class K3ListView;
class KToolBarPopupAction;
class KActionMenu;

struct StandardBook {
   QString name;
   QString path;
   QString filename;
};
typedef QList<StandardBook> StandardBookList;

/**The class PhraseTreeItem is an ListViewItem for either a phrase or a phrase book.
  *@author Gunnar Schmi Dt
  */

class CheckBookItem : public Q3CheckListItem {
public:
   CheckBookItem (Q3ListViewItem *parent, Q3ListViewItem *last,
            const QString &text, const QString &name, const QString &filename);
   CheckBookItem (Q3ListView *parent, Q3ListViewItem *last,
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

   K3ListView *books;
};

/**
 * The class StandardPhraseBookInsertAction implements an Action for
 * inserting a standard phrase book.
 * @author Gunnar Schmi Dt
 */
class StandardPhraseBookInsertAction : public KAction {
   Q_OBJECT
public:
   StandardPhraseBookInsertAction (const KUrl &url, const QString& name, const QObject* receiver, const char* slot, KActionCollection* parent)
   : KAction (KIcon("phrasebook"), name, parent, name) {
      this->url = url;
      connect (this, SIGNAL(slotActivated (const KUrl &)), receiver, slot);
   };
   ~StandardPhraseBookInsertAction () {
   };

public slots:
   void slotActivated () {
      KAction::slotTriggered();
      emit slotActivated (url);
   };

signals:
   void slotActivated (const KUrl &url);

private:
   KUrl url;
};

/**
 * This class represents a widget holding the buttons of the phrase book
 * edit window.
 * @author Gunnar Schmi Dt
 */
class ButtonBoxWidget : public QWidget, public Ui::ButtonBoxUI {
public:
   ButtonBoxWidget (QWidget *parent = 0, const char *name = 0);
   ~ButtonBoxWidget ();

   KKeyButton *keyButton;
   Q3ButtonGroup *group;

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

   static QString displayPath (QString path);

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
   
   void selectionChanged ();
   void contextMenuRequested(Q3ListViewItem *, const QPoint &pos, int);

   void slotRemove ();
   void slotCut ();
   void slotCopy ();
   void slotPaste ();

   void slotAddPhrasebook ();
   void slotAddPhrase ();

   void slotSave ();
   void slotImportPhrasebook ();
   void slotImportPhrasebook (const KUrl &url);
   void slotExportPhrasebook ();
   void slotPrint ();

   void slotDropped (QDropEvent *e, Q3ListViewItem *parent, Q3ListViewItem *after);
   void slotMoved (Q3ListViewItem *item, Q3ListViewItem *, Q3ListViewItem *);

signals:
   void phrasebookConfirmed (PhraseBook &book);

private:
   static StandardBookList standardPhraseBooks ();

   void initGUI();
   /** initializes the KActions of the window */
   void initActions();
   /** initializes the list of standard phrase books */
   void initStandardPhraseBooks ();
   
   Q3ListViewItem *addBook (Q3ListViewItem *parent, Q3ListViewItem *after, PhraseBook *book);
   Q3ListViewItem *addBook (Q3ListViewItem *item, PhraseBook *book);
   
   void setShortcut (const KShortcut &cut);

   void _warning (const QKeySequence &cut, const QString &sAction, const QString &sTitle);
   
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
