/***************************************************************************
                          phrasebookdialog.cpp  -  description
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

#include "phrasebookdialog.h"
#include "phrasetree.h"

// include files for Qt
#include <QtGui/QApplication>
#include <QtGui/QLayout>
#include <QtGui/QClipboard>
#include <QtGui/QRadioButton>
#include <QtCore/QEvent>
#include <QtGui/QPainter>
#include <QtGui/QPrintDialog>
#include <QtGui/QStyle>
#include <Qt3Support/Q3GroupBox>
#include <QtGui/QMenu>
#include <Qt3Support/Q3ValueStack>
#include <Qt3Support/Q3PtrStack>
#include <QtGui/QGridLayout>
#include <QtGui/QDropEvent>
#include <QtGui/QLabel>
#include <QtGui/QVBoxLayout>
#include <Qt3Support/Q3ListViewItem>

// include files for KDE
#include <ktoolbarpopupaction.h>
#include <kmenu.h>
#include <kxmlguifactory.h>
#include <klocale.h>
#include <kaction.h>
#include <kapplication.h>
#include <k3listview.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kactionmenu.h>
#include <kstandardaction.h>
#include <kdeprintdialog.h>

namespace PhraseBookPrivate {
   enum columns {
      name     = 1,
      filename = 2
   };
}

CheckBookItem::CheckBookItem (Q3ListViewItem *parent, Q3ListViewItem *last,
             const QString &text, const QString &name, const QString &filename)
   : Q3CheckListItem (parent, text, Q3CheckListItem::CheckBox)
{
   moveItem (last);
   setText(PhraseBookPrivate::name, name);
   setText(PhraseBookPrivate::filename, filename);
   setSelectable(false);

   if (filename.isNull() || filename.isEmpty())
      numberOfBooks = 0;
   else
      numberOfBooks = 1;
   selectedBooks = 0;
   ((CheckBookItem*)parent)->childChange (numberOfBooks, selectedBooks);
}

CheckBookItem::CheckBookItem (Q3ListView *parent, Q3ListViewItem *last,
             const QString &text, const QString &name, const QString &filename)
   : Q3CheckListItem (parent, text, Q3CheckListItem::CheckBox)
{
   moveItem (last);
   setText(PhraseBookPrivate::name, name);
   setText(PhraseBookPrivate::filename, filename);
   setSelectable(false);

   if (filename.isNull() || filename.isEmpty())
      numberOfBooks = 0;
   else
      numberOfBooks = 1;
   selectedBooks = 0;
}

CheckBookItem::~CheckBookItem () {
}

void CheckBookItem::activate() {
   Q3ListView *lv = listView();

   if ((lv != 0) && (!lv->isEnabled()) || (!isEnabled()))
      return;

   setOn (!isOn());
   ignoreDoubleClick();
}

void CheckBookItem::stateChange (bool on) {
   Q3ListViewItem *item = firstChild();
   if (item == 0) {
      Q3ListViewItem *parent = this->parent();
      if (parent != 0) {
         if (on)
            ((CheckBookItem*)parent)->childChange (0, 1);
         else
            ((CheckBookItem*)parent)->childChange (0, -1);
      }
   }
   else propagateStateChange();
}

void CheckBookItem::propagateStateChange () {
   Q3ListViewItem *item = firstChild();
   while (item != 0) {
      if (isOn() != ((Q3CheckListItem*)item)->isOn())
         ((Q3CheckListItem*)item)->setOn (isOn());
      else
         ((CheckBookItem*)item)->propagateStateChange ();
      item = item->nextSibling();
   }
}

void CheckBookItem::childChange (int numberDiff, int selDiff) {
   numberOfBooks += numberDiff;
   selectedBooks += selDiff;
   Q3ListViewItem *parent = this->parent();
   if (parent != 0)
      ((CheckBookItem*)parent)->childChange (numberDiff, selDiff);

   QString text = i18np(" (%2 of 1 book selected)",
                        " (%2 of %1 books selected)",
                        numberOfBooks, selectedBooks);
   setText(0, this->text(PhraseBookPrivate::name)+text);
}

/***************************************************************************/

InitialPhraseBookWidget::InitialPhraseBookWidget (QWidget *parent, const char *name)
   : QWidget(parent)
{
   setObjectName( QLatin1String( name ) );
   QVBoxLayout *mainLayout = new QVBoxLayout (this);
   mainLayout->setSpacing(KDialog::spacingHint());
   QLabel *label = new QLabel (i18n("Please decide which phrase books you need:"), this);
   label->setObjectName( QLatin1String("booksTitle" ));
   mainLayout->addWidget (label);

   books = new K3ListView (this);
   books->setSorting (-1);
   books->setItemsMovable (false);
   books->setDragEnabled (false);
   books->setAcceptDrops (false);
   books->addColumn (i18n("Book"));
   books->setRootIsDecorated (true);
   books->setAllColumnsShowFocus (true);
   books->setSelectionMode (Q3ListView::Multi);
   mainLayout->addWidget (books);

   initStandardPhraseBooks();
}

InitialPhraseBookWidget::~InitialPhraseBookWidget () {
}

void InitialPhraseBookWidget::initStandardPhraseBooks() {
   StandardBookList bookPaths = PhraseBookDialog::standardPhraseBooks();

   Q3ListViewItem *parent = 0;
   Q3ListViewItem *last = 0;
   QStringList currentNamePath;
   currentNamePath<<QLatin1String("");
   Q3PtrStack<Q3ListViewItem> stack;
   StandardBookList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      QString namePath = (*it).path;
      QStringList dirs = namePath.split( QLatin1Char( '/' ));

      QStringList::iterator it1=currentNamePath.begin();
      QStringList::iterator it2=dirs.begin();
      for (; (it1 != currentNamePath.end())
          && (it1 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);
      for (; it1 != currentNamePath.end(); ++it1) {
         last = parent;
         parent = stack.pop();
      }
      for (; it2 != dirs.end(); ++it2) {
         stack.push (parent);
         Q3ListViewItem *newParent;
         if (parent == 0)
            newParent = new CheckBookItem (books, last, *it2, *it2, QString());
         else
            newParent = new CheckBookItem (parent, last, *it2, *it2, QString());
         parent = newParent;
         last = 0;
      }
      currentNamePath = dirs;

      Q3ListViewItem *book;
      if (parent == 0)
         book = new CheckBookItem (books, last, (*it).name, (*it).name, (*it).filename);
      else
         book = new CheckBookItem (parent, last, (*it).name, (*it).name, (*it).filename);
      last = book;
   }
}

void InitialPhraseBookWidget::createBook () {
   PhraseBook book;
   Q3ListViewItem *item = books->firstChild();
   while (item != 0) {
      if (item->firstChild() != 0) {
         item = item->firstChild();
      }
      else {
         if (((Q3CheckListItem*)item)->isOn()) {
            PhraseBook localBook;
            localBook.open(KUrl( item->text(PhraseBookPrivate::filename )));
            book += localBook;
         }

         while ((item != 0) && (item->nextSibling() == 0)) {
            item = item->parent();
         }
         if (item != 0)
            item = item->nextSibling();
      }
   }

   QString bookLocation = KGlobal::dirs()->saveLocation ("appdata", QLatin1String( "/" ));
   if (!bookLocation.isNull() && !bookLocation.isEmpty()) {
      book.save (KUrl( bookLocation + QLatin1String( "standard.phrasebook" ) ));
   }
}

/***************************************************************************/

ButtonBoxWidget::ButtonBoxWidget (QWidget *parent, const char *name)
: QWidget(parent)
{
   setupUi(this);
   setObjectName( QLatin1String( name ) );
   keyButtonPlaceLayout = new QGridLayout (keyButtonPlace);
   keyButtonPlaceLayout->setObjectName( QLatin1String("keyButtonPlaceLayout" ));
   keyButtonPlaceLayout->setMargin(0);
   keyButtonPlaceLayout->setSpacing(0);

   keyButton = new KKeySequenceWidget (keyButtonPlace);
   keyButtonPlaceLayout->addWidget (keyButton, 1,1);
   keyButton->setWhatsThis( i18n("By clicking on this button you can select the keyboard shortcut associated with the selected phrase."));

   group = new Q3ButtonGroup (phrasebox);
   group->hide();
   group->setExclusive (true);
   group->insert (noKey);
   group->insert (customKey);
}

ButtonBoxWidget::~ButtonBoxWidget () {
}

/***************************************************************************/

namespace PhraseBookPrivate {
   PhraseBookDialog *instance = 0;
}

PhraseBookDialog::PhraseBookDialog ()
 : KXmlGuiWindow (0)
{
   setObjectName( QLatin1String("phraseEditDialog" ));
   setCaption (i18n("Phrase Book"));
   initGUI();
   initActions();
   initStandardPhraseBooks();
   QString standardBook = KGlobal::dirs()->findResource("appdata", QLatin1String( "standard.phrasebook" ));
   if (!standardBook.isNull() && !standardBook.isEmpty()) {
      PhraseBook book;
      book.open(KUrl( standardBook ));
      treeView->clear();
      treeView->addBook(0, 0, &book);
      treeView->setCurrentItem(treeView->firstChild());
      selectionChanged();
      phrasebookChanged = false;
   }

   printer = 0;
   // i18n("Edit Phrase Book")
}

PhraseBookDialog *PhraseBookDialog::get() {
   if (PhraseBookPrivate::instance == 0)
      PhraseBookPrivate::instance = new PhraseBookDialog();
   return PhraseBookPrivate::instance;
}

PhraseBookDialog::~PhraseBookDialog() {
   PhraseBookPrivate::instance = 0;
   delete printer;
}

void PhraseBookDialog::initGUI () {
   QWidget *page = new QWidget( this );
   setCentralWidget(page);
   QVBoxLayout *mainLayout = new QVBoxLayout (page);
   mainLayout->setMargin(0);

   treeView = new PhraseTree (page, "phrasetree");
   treeView->setSorting (-1);
   treeView->setItemsMovable (true);
   treeView->setDragEnabled (true);
   treeView->setAcceptDrops (true);
   treeView->addColumn (i18n("Phrase"));
   treeView->addColumn (i18n("Shortcut"));
   treeView->setRootIsDecorated (true);
   treeView->setAllColumnsShowFocus (true);
   treeView->setSelectionMode (Q3ListView::Extended);
   treeView->setWhatsThis( i18n("This list contains the current phrase book in a tree structure. You can select and modify individual phrases and sub phrase books"));
   connect (treeView, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
   connect (treeView, SIGNAL(contextMenuRequested (Q3ListViewItem *, const QPoint &, int)), this, SLOT(contextMenuRequested (Q3ListViewItem *, const QPoint &, int)));
   connect (treeView, SIGNAL(dropped (QDropEvent *, Q3ListViewItem *, Q3ListViewItem *)), this, SLOT(slotDropped (QDropEvent *, Q3ListViewItem *, Q3ListViewItem *)));
   connect (treeView, SIGNAL(moved (Q3ListViewItem *, Q3ListViewItem *, Q3ListViewItem *)), this, SLOT(slotMoved (Q3ListViewItem *, Q3ListViewItem *, Q3ListViewItem *)));
   mainLayout->addWidget (treeView);

   buttonBox = new ButtonBoxWidget (page, "buttonbox");
   connect (buttonBox->lineEdit, SIGNAL(textChanged(const QString &)), SLOT(slotTextChanged(const QString &)));
   connect (buttonBox->noKey, SIGNAL(clicked()), SLOT(slotNoKey()));
   connect (buttonBox->customKey, SIGNAL(clicked()), SLOT(slotCustomKey()));
   connect (buttonBox->keyButton, SIGNAL(capturedShortcut(const KShortcut&)), SLOT(capturedShortcut(const KShortcut&)));
   mainLayout->addWidget (buttonBox);

   treeView->setFocus();
   selectionChanged();
}

void PhraseBookDialog::initActions() {
// The file menu
   fileNewPhrase = actionCollection()->addAction(QLatin1String( "file_new_phrase" ));
   fileNewPhrase->setIcon(KIcon( QLatin1String( "phrase_new" )));
   fileNewPhrase->setText(i18n("&New Phrase"));
   connect(fileNewPhrase, SIGNAL(triggered(bool)), this, SLOT(slotAddPhrase()));
   fileNewPhrase->setToolTip(i18n("Adds a new phrase"));
   fileNewPhrase->setWhatsThis (i18n("Adds a new phrase"));

   fileNewBook = actionCollection()->addAction(QLatin1String( "file_new_book" ));
   fileNewBook->setIcon(KIcon( QLatin1String( "phrasebook_new" )));
   fileNewBook->setText(i18n("New Phrase &Book"));
   connect(fileNewBook, SIGNAL(triggered(bool)), this, SLOT(slotAddPhrasebook()));
   fileNewBook->setToolTip(i18n("Adds a new phrase book into which other books and phrases can be placed"));
   fileNewBook->setWhatsThis (i18n("Adds a new phrase book into which other books and phrases can be placed"));

   fileSave = KStandardAction::save(this, SLOT(slotSave()), actionCollection());
   fileSave->setToolTip(i18n("Saves the phrase book onto the hard disk"));
   fileSave->setWhatsThis (i18n("Saves the phrase book onto the hard disk"));

   fileImport = actionCollection()->addAction(QLatin1String( "file_import" ));
   fileImport->setIcon(KIcon( QLatin1String( "phrasebook_open" )));
   fileImport->setText(i18n("&Import..."));
   connect(fileImport, SIGNAL(triggered(bool)), this, SLOT(slotImportPhrasebook()));
   fileImport->setToolTip(i18n("Imports a file and adds its contents to the phrase book"));
   fileImport->setWhatsThis (i18n("Imports a file and adds its contents to the phrase book"));

   toolbarImport = new KToolBarPopupAction(KIcon( QLatin1String( "phrasebook_open") ), i18n("&Import..." ), this);
   actionCollection()->addAction(QLatin1String( "toolbar_import" ), toolbarImport);
   connect(toolbarImport, SIGNAL(triggered(bool)), this, SLOT(slotImportPhrasebook()));
   toolbarImport->setToolTip(i18n("Imports a file and adds its contents to the phrase book"));
   toolbarImport->setWhatsThis (i18n("Imports a file and adds its contents to the phrase book"));

   fileImportStandardBook = actionCollection()->add<KActionMenu>(QLatin1String( "file_import_standard_book" ));
   fileImportStandardBook->setIcon(KIcon( QLatin1String( "phrasebook_open" )));
   fileImportStandardBook->setText(i18n("I&mport Standard Phrase Book"));
   fileImportStandardBook->setToolTip(i18n("Imports a standard phrase book and adds its contents to the phrase book"));
   fileImportStandardBook->setWhatsThis (i18n("Imports a standard phrase book and adds its contents to the phrase book"));

   fileExport = actionCollection()->addAction(QLatin1String( "file_export" ));
   fileExport->setIcon(KIcon( QLatin1String( "phrasebook_save" )));
   fileExport->setText(i18n("&Export..."));
   connect(fileExport, SIGNAL(triggered(bool)), this, SLOT(slotExportPhrasebook()));
   fileExport->setToolTip(i18n("Exports the currently selected phrase(s) or phrase book(s) into a file"));
   fileExport->setWhatsThis (i18n("Exports the currently selected phrase(s) or phrase book(s) into a file"));

   filePrint = KStandardAction::print(this, SLOT(slotPrint()), actionCollection());
   filePrint->setToolTip(i18n("Prints the currently selected phrase(s) or phrase book(s)"));
   filePrint->setWhatsThis (i18n("Prints the currently selected phrase(s) or phrase book(s)"));

   fileClose = KStandardAction::close(this, SLOT(close()), actionCollection());
   fileClose->setToolTip(i18n("Closes the window"));
   fileClose->setWhatsThis (i18n("Closes the window"));

// The edit menu
   editCut = KStandardAction::cut(this, SLOT(slotCut()), actionCollection());
   editCut->setToolTip(i18n("Cuts the currently selected entries from the phrase book and puts it to the clipboard"));
   editCut->setWhatsThis (i18n("Cuts the currently selected entries from the phrase book and puts it to the clipboard"));

   editCopy = KStandardAction::copy(this, SLOT(slotCopy()), actionCollection());
   editCopy->setToolTip(i18n("Copies the currently selected entry from the phrase book to the clipboard"));
   editCopy->setWhatsThis (i18n("Copies the currently selected entry from the phrase book to the clipboard"));

   editPaste = KStandardAction::paste(this, SLOT(slotPaste()), actionCollection());
   editPaste->setToolTip(i18n("Pastes the clipboard contents to actual position"));
   editPaste->setWhatsThis (i18n("Pastes the clipboard contents to actual position"));

   editDelete = actionCollection()->addAction(QLatin1String( "edit_delete" ));
   editDelete->setIcon(KIcon( QLatin1String( "edit-delete" )));
   editDelete->setText(i18n("&Delete"));
   connect(editDelete, SIGNAL(triggered(bool)), this, SLOT(slotRemove()));
   editDelete->setToolTip(i18n("Deletes the selected entries from the phrase book"));
   editDelete->setWhatsThis (i18n("Deletes the selected entries from the phrase book"));

   // use the absolute path to your kmouthui.rc file for testing purpose in createGUI();
   createGUI(QLatin1String( "phrasebookdialogui.rc" ));
}

QString PhraseBookDialog::displayPath (QString filename) {
   QFileInfo file(filename);
   QString path = file.path();
   QString dispPath;
   int position = path.indexOf(QLatin1String( "/kmouth/books/" ))+QString( QLatin1String("/kmouth/books/") ).length();

   while (path.length() > position) {
      file.setFile(path);

      KDesktopFile *dirDesc = new KDesktopFile("data", path+QLatin1String( "/.directory" ));
      QString name = dirDesc->readName();
      delete dirDesc;

      if (name.isNull() || name.isEmpty())
         dispPath += QLatin1Char( '/' ) + file.fileName ();
      else
         dispPath += QLatin1Char( '/' ) + name;

      path = file.path();
   }
   return dispPath;
}

StandardBookList PhraseBookDialog::standardPhraseBooks() {
   QStringList bookPaths = KGlobal::mainComponent().dirs()->findAllResources (
                          "data", QLatin1String( "kmouth/books/*.phrasebook" ),
                          KStandardDirs::Recursive |
                          KStandardDirs::NoDuplicates);
   QStringList bookNames;
   QMap<QString,StandardBook> bookMap;
   QStringList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      PhraseBook pbook;
      if (pbook.open (KUrl( *it ))) {
         StandardBook book;
         book.name = (*pbook.begin()).getPhrase().getPhrase();

         book.path = displayPath(*it);
         book.filename = *it;

         bookNames += book.path + QLatin1Char( '/' ) + book.name;
         bookMap [book.path + QLatin1Char( '/' ) + book.name] = book;
      }
   }

   bookNames.sort();

   StandardBookList result;
   for (it = bookNames.begin(); it != bookNames.end(); ++it)
      result += bookMap [*it];

   return result;
}

void PhraseBookDialog::initStandardPhraseBooks () {
   StandardBookList bookPaths = standardPhraseBooks();

   KActionMenu *parent = fileImportStandardBook;
   QStringList currentNamePath;
   currentNamePath<< QLatin1String( "x" );
   Q3PtrStack<KActionMenu> stack;
   StandardBookList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      KUrl url;
      url.setPath((*it).filename);

      QString namePath = QLatin1String( "x/" )+(*it).path;
      QStringList dirs = namePath.split(QLatin1Char(  '/' ));

      QStringList::iterator it1=currentNamePath.begin();
      QStringList::iterator it2=dirs.begin();
      for (; (it1 != currentNamePath.end())
          && (it1 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);
      for (; it1 != currentNamePath.end(); ++it1)
         parent = stack.pop();
      for (; it2 != dirs.end(); ++it2) {
         stack.push (parent);
#ifdef __GNUC__
#warning "kde4: correct newparent objectname ?"
#endif
         KActionMenu *newParent = actionCollection()->add<KActionMenu>(QLatin1String( "tmp_menu" ));
         newParent->setText(*it2);
         parent->addAction(newParent);
         if (parent == fileImportStandardBook)
            toolbarImport->popupMenu()->addAction(newParent);
         parent = newParent;
      }
      currentNamePath = dirs;

      KAction *book = new StandardPhraseBookInsertAction (
          url, (*it).name, this, SLOT(slotImportPhrasebook (const KUrl &)), actionCollection());
      parent->addAction(book);
      if (parent == fileImportStandardBook)
         toolbarImport->popupMenu()->addAction(book);
   }
}

PhraseTreeItem *selectedItem (Q3ListView *treeView) {
   PhraseTreeItem *currentItem = (PhraseTreeItem *)treeView->currentItem();
   if ((currentItem == 0) || (!currentItem->isSelected()))
      return 0;

   Q3ListViewItemIterator it(treeView);
   while (it.current()) {
      Q3ListViewItem *item = it.current();
      if (item->isSelected() && (item != currentItem))
         return 0;
      ++it;
   }
   return currentItem;
}

void PhraseBookDialog::selectionChanged () {
   bool modified = phrasebookChanged;
   PhraseTreeItem *currentItem = selectedItem (treeView);
   if (currentItem == 0) {
      buttonBox->textLabel->setText (i18n("Text of the &phrase:"));
      buttonBox->textLabel->setEnabled(false);
      buttonBox->group->setEnabled(false);
      buttonBox->lineEdit->setText(QLatin1String( "" ));
      buttonBox->lineEdit->setEnabled(false);
      buttonBox->shortcutLabel->setEnabled(false);
      //buttonBox->keyButton->setShortcut("", false);
      buttonBox->keyButton->setEnabled(false);
      buttonBox->noKey->setChecked (false);
      buttonBox->noKey->setEnabled (false);
      buttonBox->customKey->setChecked (false);
      buttonBox->customKey->setEnabled (false);
   }
   else if (currentItem->isPhrase()) {
      buttonBox->textLabel->setText (i18n("Text of the &phrase:"));
      buttonBox->textLabel->setEnabled(true);
      buttonBox->group->setEnabled(true);
      buttonBox->lineEdit->setText(currentItem->text(0));
      buttonBox->lineEdit->setEnabled(true);
      buttonBox->shortcutLabel->setEnabled(true);
      QString shortcut = currentItem->text(1);
#ifdef __GNUC__
#warning "kde4 port it"
#endif
      //buttonBox->keyButton->setShortcut(currentItem->cut(), false);
      if (shortcut.isEmpty() || shortcut.isNull()) {
         buttonBox->noKey->setChecked (true);
         buttonBox->customKey->setChecked (false);
      }
      else {
         buttonBox->noKey->setChecked (false);
         buttonBox->customKey->setChecked (true);
      }
      buttonBox->keyButton->setEnabled(true);
      buttonBox->noKey->setEnabled(true);
      buttonBox->customKey->setEnabled(true);
   }
   else {
      buttonBox->textLabel->setText (i18n("Name of the &phrase book:"));
      buttonBox->textLabel->setEnabled(true);
      buttonBox->group->setEnabled(true);
      buttonBox->lineEdit->setText(currentItem->text(0));
      buttonBox->lineEdit->setEnabled(true);
      buttonBox->shortcutLabel->setEnabled(false);
      //buttonBox->keyButton->setShortcut("", false);
      buttonBox->keyButton->setEnabled(false);
      buttonBox->noKey->setChecked (false);
      buttonBox->noKey->setEnabled (false);
      buttonBox->customKey->setChecked (false);
      buttonBox->customKey->setEnabled (false);
   }
   phrasebookChanged = modified;
}

bool PhraseBookDialog::queryClose() {
   if (phrasebookChanged) {
      int answer = KMessageBox::questionYesNoCancel (this,
              i18n("<qt>There are unsaved changes.<br />Do you want to apply the changes before closing the \"phrase book\" window or discard the changes?</qt>"),
              i18n("Closing \"Phrase Book\" Window"),
              KStandardGuiItem::apply(), KStandardGuiItem::discard(),
              KStandardGuiItem::cancel(), QLatin1String( "AutomaticSave" ));
      if (answer == KMessageBox::Yes) {
         slotSave();
         return true;
      }
      return (answer == KMessageBox::No);
   }
   return true;
}

void PhraseBookDialog::slotTextChanged (const QString &s) {
   PhraseTreeItem *currentItem = selectedItem (treeView);
   if (currentItem != 0)
      currentItem->setText(0, s);
   phrasebookChanged = true;
}

void PhraseBookDialog::slotNoKey() {
   buttonBox->noKey->setChecked (true);
   buttonBox->customKey->setChecked (false);

   PhraseTreeItem *currentItem = selectedItem (treeView);
   if (currentItem != 0) {
      currentItem->setCut (KShortcut(QString()));
#ifdef __GNUC__
#warning "kde4: port it"
#endif
	  //buttonBox->keyButton->setShortcut(currentItem->cut(), false);
   }
   phrasebookChanged = true;
}

void PhraseBookDialog::slotCustomKey() {
   buttonBox->keyButton->keySequence();
}

void PhraseBookDialog::capturedShortcut (const KShortcut& cut) {
   if (cut.isEmpty()) {
      slotNoKey();
   }
   else
      setShortcut (cut);
   phrasebookChanged = true;
}

void PhraseBookDialog::setShortcut( const KShortcut& cut ) {
   // Check whether the shortcut is valid
   const QList<QKeySequence> cutList = cut.toList();
   for (int i = 0; i < cutList.count(); i++) {
      const QKeySequence& seq = cutList[i];
      //const KKey& key = seq.key(0);
#ifdef __GNUC__
#warning "kde 4 port it";
#endif
#if 0
      if (key.modFlags() == 0 && key.sym() < 0x3000
          && QChar(key.sym()).isLetterOrNumber())
      {
         QString s = i18n("In order to use the '%1' key as a shortcut, "
                          "it must be combined with the "
                          "Win, Alt, Ctrl, and/or Shift keys.", QChar(key.sym()));
         KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
         return;
      }
#endif
   }
   PhraseTreeItem *currentItem = selectedItem (treeView);
   // If key isn't already in use,
   if (!treeView->isKeyPresent (cut, currentItem, true)) {
      // Set new key code
      currentItem->setCut (cut);
      // Update display
      buttonBox->noKey->setChecked (false);
      buttonBox->customKey->setChecked (true);
      buttonBox->keyButton->setKeySequence(currentItem->cut().primary());
   }
}

Q3ListViewItem *PhraseBookDialog::addBook (Q3ListViewItem *parent, Q3ListViewItem *after, PhraseBook *book) {
   Q3ListViewItem *newItem = treeView->addBook(parent, after, book);
   if (newItem != 0) {
      treeView->clearSelection();
      treeView->ensureItemVisible(newItem);
      treeView->setCurrentItem (newItem);
      treeView->setSelected (newItem, true);
      phrasebookChanged = true;
   }
   return newItem;
}

Q3ListViewItem *PhraseBookDialog::addBook (Q3ListViewItem *item, PhraseBook *book) {
   if (item == 0)
      return addBook(0, 0, book);
   else if (((PhraseTreeItem *)item)->isPhrase() || !item->isOpen())
      if (item->parent() != 0)
         return addBook(item->parent(), item, book);
      else
         return addBook(0, item, book);
   else
      return addBook(item, 0, book);
}

void PhraseBookDialog::contextMenuRequested(Q3ListViewItem *, const QPoint &pos, int) {
   QString name;
   if (treeView->hasSelectedItems())
      name = QLatin1String( "phrasebook_popup_sel" );
   else
      name = QLatin1String( "phrasebook_popup_nosel" );

   QMenu *popup = (QMenu *)factory()->container(name,this);
   if (popup != 0) {
      popup->popup(pos, 0);
   }
}

void PhraseBookDialog::slotRemove () {
   if (treeView->hasSelectedItems() != 0)
      treeView->deleteSelectedItems();
   selectionChanged();
   phrasebookChanged = true;
}

void PhraseBookDialog::slotCut () {
   slotCopy();
   slotRemove();
}

void PhraseBookDialog::slotCopy () {
   PhraseBook book;
   treeView->fillBook (&book, true);
#ifdef __GNUC__
#warning "kde4: port to QMimeData"
#endif
   QApplication::clipboard()->setData(new PhraseBookDrag(&book));
}

void PhraseBookDialog::slotPaste () {
   PhraseBook book;
   if (PhraseBookDrag::decode(QApplication::clipboard()->data(), &book)) {
      addBook (treeView->currentItem(), &book);
   }
}

void PhraseBookDialog::slotDropped (QDropEvent *e, Q3ListViewItem *parent, Q3ListViewItem *after) {
   PhraseBook book;
   if (PhraseBookDrag::decode(e, &book)) {
      addBook(parent, after, &book);
   }
}

void PhraseBookDialog::slotMoved (Q3ListViewItem *item, Q3ListViewItem *, Q3ListViewItem *) {
   treeView->ensureItemVisible(item);
   treeView->setSelected (item, true);
   phrasebookChanged = true;
}

void PhraseBookDialog::slotAddPhrasebook () {
   PhraseBook book;
   Phrase phrase(i18n("(New Phrase Book)"), QLatin1String( "" ));
   book += PhraseBookEntry(phrase, 0, false);

   Q3ListViewItem *item = addBook (treeView->currentItem(), &book);
   item->setOpen (true);
   buttonBox->lineEdit->selectAll();
   buttonBox->lineEdit->setFocus();
}

void PhraseBookDialog::slotAddPhrase () {
   PhraseBook book;
   Phrase phrase(i18n("(New Phrase)"), QLatin1String( "" ));
   book += PhraseBookEntry(phrase, 0, true);

   addBook (treeView->currentItem(), &book);
   buttonBox->lineEdit->selectAll();
   buttonBox->lineEdit->setFocus();
}

void PhraseBookDialog::slotSave () {
   book.clear();
   treeView->fillBook (&book, false);
   emit phrasebookConfirmed (book);
   phrasebookChanged = false;
}

void PhraseBookDialog::slotImportPhrasebook () {
   KUrl url=KFileDialog::getOpenUrl(KUrl(),
        i18n("*.phrasebook|Phrase Books (*.phrasebook)\n*.txt|Plain Text Files (*.txt)\n*|All Files"), this, i18n("Import Phrasebook"));

   slotImportPhrasebook (url);
}

void PhraseBookDialog::slotImportPhrasebook (const KUrl &url) {
   if(!url.isEmpty()) {
      PhraseBook book;
      if (book.open (url))
         addBook(treeView->currentItem(), &book);
      else
         KMessageBox::sorry(this,i18n("There was an error loading file\n%1", url.url() ));
   }
}

void PhraseBookDialog::slotExportPhrasebook () {
   PhraseBook book;
   treeView->fillBook (&book, treeView->hasSelectedItems());

   KUrl url;
   if (book.save (this, i18n("Export Phrase Book"), url) == -1)
      KMessageBox::sorry(this,i18n("There was an error saving file\n%1", url.url() ));
}

void PhraseBookDialog::slotPrint()
{
   if (printer == 0) {
     printer = new QPrinter();
   }

   QPrintDialog *printDialog = KdePrint::createPrintDialog(printer, this);

   if (printDialog->exec()) {
      PhraseBook book;
      treeView->fillBook (&book, treeView->hasSelectedItems());

      book.print(printer);
   }
   delete printDialog;
}

#include "phrasebookdialog.moc"
