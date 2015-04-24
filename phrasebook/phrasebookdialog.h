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

#include <Qt3Support/Q3ListView>
#include <QDomNode>
#include <QStandardItemModel>
#include <QWizardPage>

#include <kurl.h>
#include <kxmlguiwindow.h>

#include "phrasebook.h"
#include "ui_phrasebookdialog.h"

#include <QDebug>

class Q3ListViewItem;
class PhraseTreeItem;
class QTreeView;
class QString;
class K3ListView;
class KToolBarPopupAction;
class KActionMenu;
class PhraseBookModel;
class PhraseBookItem;

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
class InitialPhraseBookWidget : public QWizardPage {
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
   : KAction (KIcon(QLatin1String( "phrasebook" )), name, parent) {
      this->url = url;
      connect (this, SIGNAL(triggered(bool)), this, SLOT(slotActivated()));
      connect (this, SIGNAL(slotActivated (const KUrl &)), receiver, slot);
      parent->addAction(name, this);
   }
   ~StandardPhraseBookInsertAction () {
   }

public slots:
   void slotActivated () {
      emit slotActivated (url);
   }

signals:
   void slotActivated (const KUrl &url);

private:
   KUrl url;
};

/**
 * The class PhraseBookDialog implements a dialog for editing phrase books.
 * @author Gunnar Schmi Dt
 */

class PhraseBookDialog : public KXmlGuiWindow {
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
   void slotKeySequenceChanged (const QKeySequence& sequence);

   void selectionChanged ();
   void contextMenuRequested(const QPoint &pos);

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
   //void slotPrint ();

   void slotModelChanged();

signals:
   void phrasebookConfirmed ();

private:
   static StandardBookList standardPhraseBooks ();

   void initGUI();
   /** initializes the KActions of the window */
   void initActions();
   /** initializes the list of standard phrase books */
   void initStandardPhraseBooks ();

   void connectEditor();
   void disconnectEditor();

   // Deserialize the book from the given QDomNode and under the given parent
   // Return the new QStandardItem so it can get focused.
   QStandardItem* deserializeBook(const QDomNode &node, QStandardItem *parent);
   // Return a serialized string of the book or phrase at the given index.
   QString serializeBook(const QModelIndex &index);

   // Get the current parent, if the current index is not a book, get it's parent.
   QModelIndex getCurrentParent();

   // Expand to, select and focus a new item from the given parameters
   void focusNewItem(QModelIndex parent, QStandardItem *item);

   void setShortcut (const QKeySequence &sequence);

   void _warning (const QKeySequence &cut, const QString &sAction, const QString &sTitle);

   bool isGlobalKeyPresent (const KShortcut& cut, bool warnUser);
   bool isPhraseKeyPresent (const KShortcut& cut, bool warnUser);
   bool isKeyPresent (const KShortcut& cut, bool warnUser);

   bool phrasebookChanged;

   QAction* fileNewPhrase;
   QAction* fileNewBook;
   QAction* fileSave;
   QAction* fileImport;
   KToolBarPopupAction* toolbarImport;
   KActionMenu* fileImportStandardBook;
   QAction* fileExport;
   //QAction* filePrint;
   QAction* fileClose;
   QAction* editCut;
   QAction* editCopy;
   QAction* editPaste;
   QAction* editDelete;

   QStandardItemModel* m_bookModel;
   QStandardItem *m_rootItem;

   // Keep QPrinter so settings persist
   //QPrinter *printer;

   Ui::PhraseBookDialog *m_ui;
};

#endif
