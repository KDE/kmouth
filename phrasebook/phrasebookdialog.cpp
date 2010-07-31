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

// include files for QT
#include <tqapplication.h>
#include <tqlayout.h>
#include <tqclipboard.h>
#include <tqradiobutton.h>
#include <tqevent.h>
#include <tqpainter.h>
#include <tqstyle.h>
#include <tqgroupbox.h>
#include <tqpopupmenu.h>
#include <tqvaluestack.h>
#include <tqptrstack.h>
#include <tqwhatsthis.h>

// include files for KDE
#include <kpopupmenu.h>
#include <klocale.h>
#include <kaction.h>
#include <kapplication.h>
#include <klistview.h>
#include <kiconloader.h>
#include <kguiitem.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h> 

#include "phrasebookdialog.h"
#include "buttonboxui.h"
#include "phrasetree.h"

namespace PhraseBookPrivate {
   enum columns {
      name     = 1,
      filename = 2
   };
}

CheckBookItem::CheckBookItem (TQListViewItem *parent, TQListViewItem *last,
             const TQString &text, const TQString &name, const TQString &filename)
   : TQCheckListItem (parent, text, TQCheckListItem::CheckBox)
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

CheckBookItem::CheckBookItem (TQListView *parent, TQListViewItem *last,
             const TQString &text, const TQString &name, const TQString &filename)
   : TQCheckListItem (parent, text, TQCheckListItem::CheckBox)
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
   TQListView *lv = listView();

   if ((lv != 0) && (!lv->isEnabled()) || (!isEnabled()))
      return;

   setOn (!isOn());
   ignoreDoubleClick();
}

void CheckBookItem::stateChange (bool on) {
   TQListViewItem *item = firstChild();
   if (item == 0) {
      TQListViewItem *parent = this->parent();
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
   TQListViewItem *item = firstChild();
   while (item != 0) {
      if (isOn() != ((TQCheckListItem*)item)->isOn())
         ((TQCheckListItem*)item)->setOn (isOn());
      else
         ((CheckBookItem*)item)->propagateStateChange ();
      item = item->nextSibling();
   }
}

void CheckBookItem::childChange (int numberDiff, int selDiff) {
   numberOfBooks += numberDiff;
   selectedBooks += selDiff;
   TQListViewItem *parent = this->parent();
   if (parent != 0)
      ((CheckBookItem*)parent)->childChange (numberDiff, selDiff);

   TQString text = i18n(" (%1 of %2 books selected)");
   text = text.arg(selectedBooks).arg(numberOfBooks);
   setText(0, this->text(PhraseBookPrivate::name)+text);
}

/***************************************************************************/

InitialPhraseBookWidget::InitialPhraseBookWidget (TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
   TQVBoxLayout *mainLayout = new TQVBoxLayout (this, 0, KDialog::spacingHint());
   TQLabel *label = new TQLabel (i18n("Please decide which phrase books you need:"), this, "booksTitle");
   mainLayout->add (label);

   books = new KListView (this, "books");
   books->setSorting (-1);
   books->setItemsMovable (false);
   books->setDragEnabled (false);
   books->setAcceptDrops (false);
   books->addColumn (i18n("Book"));
   books->setRootIsDecorated (true);
   books->setAllColumnsShowFocus (true);
   books->setSelectionMode (TQListView::Multi);
   mainLayout->add (books);

   initStandardPhraseBooks();
}

InitialPhraseBookWidget::~InitialPhraseBookWidget () {
}

void InitialPhraseBookWidget::initStandardPhraseBooks() {
   StandardBookList bookPaths = PhraseBookDialog::standardPhraseBooks();

   TQListViewItem *parent = 0;
   TQListViewItem *last = 0;
   TQStringList currentNamePath = "";
   TQPtrStack<TQListViewItem> stack;
   StandardBookList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      TQString namePath = (*it).path;
      TQStringList dirs = TQStringList::split("/", namePath);

      TQStringList::iterator it1=currentNamePath.begin();
      TQStringList::iterator it2=dirs.begin();
      for (; (it1 != currentNamePath.end())
          && (it1 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);
      for (; it1 != currentNamePath.end(); ++it1) {
         last = parent;
         parent = stack.pop();
      }
      for (; it2 != dirs.end(); ++it2) {
         stack.push (parent);
         TQListViewItem *newParent;
         if (parent == 0)
            newParent = new CheckBookItem (books, last, *it2, *it2, TQString::null);
         else
            newParent = new CheckBookItem (parent, last, *it2, *it2, TQString::null);
         parent = newParent;
         last = 0;
      }
      currentNamePath = dirs;
      
      TQListViewItem *book;
      if (parent == 0)
         book = new CheckBookItem (books, last, (*it).name, (*it).name, (*it).filename);
      else
         book = new CheckBookItem (parent, last, (*it).name, (*it).name, (*it).filename);
      last = book;
   }
}

void InitialPhraseBookWidget::createBook () {
   PhraseBook book;
   TQListViewItem *item = books->firstChild();
   while (item != 0) {
      if (item->firstChild() != 0) {
         item = item->firstChild();
      }
      else {
         if (((TQCheckListItem*)item)->isOn()) {
            PhraseBook localBook;
            localBook.open(KURL( item->text(PhraseBookPrivate::filename )));
            book += localBook;
         }
         
         while ((item != 0) && (item->nextSibling() == 0)) {
            item = item->parent();
         }
         if (item != 0)
            item = item->nextSibling();
      }
   }

   TQString bookLocation = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/");
   if (!bookLocation.isNull() && !bookLocation.isEmpty()) {
      book.save (KURL( bookLocation + "standard.phrasebook" ));
   }
}

/***************************************************************************/

ButtonBoxWidget::ButtonBoxWidget (TQWidget *parent, const char *name)
: ButtonBoxUI (parent, name) {
   keyButtonPlaceLayout = new TQGridLayout (keyButtonPlace, 1, 1, 0, 0, "keyButtonPlaceLayout");

   keyButton = new KKeyButton (keyButtonPlace, "key");
   keyButtonPlaceLayout->addWidget (keyButton, 1,1);
   TQWhatsThis::add (keyButton, i18n("By clicking on this button you can select the keyboard shortcut associated with the selected phrase."));

   group = new TQButtonGroup (phrasebox);
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
 : KMainWindow (0, "phraseEditDialog")
{
   setCaption (i18n("Phrase Book"));
   initGUI();
   initActions();
   initStandardPhraseBooks();
   TQString standardBook = KApplication::kApplication()->dirs()->findResource("appdata", "standard.phrasebook");
   if (!standardBook.isNull() && !standardBook.isEmpty()) {
      PhraseBook book;
      book.open(KURL( standardBook ));
      treeView->clear();
      treeView->addBook(0, 0, &book);
      treeView->setCurrentItem(treeView->firstChild());
      selectionChanged();
      phrasebookChanged = false;
   }
   // i18n("Edit Phrase Book")
}

PhraseBookDialog *PhraseBookDialog::get() {
   if (PhraseBookPrivate::instance == 0)
      PhraseBookPrivate::instance = new PhraseBookDialog();
   return PhraseBookPrivate::instance;
}

PhraseBookDialog::~PhraseBookDialog() {
   PhraseBookPrivate::instance = 0;
}

void PhraseBookDialog::initGUI () {
   TQWidget *page = new TQWidget( this );
   setCentralWidget(page);
   TQVBoxLayout *mainLayout = new TQVBoxLayout (page, 0);
   
   treeView = new PhraseTree (page, "phrasetree");
   treeView->setSorting (-1);
   treeView->setItemsMovable (true);
   treeView->setDragEnabled (true);
   treeView->setAcceptDrops (true);
   treeView->addColumn (i18n("Phrase"));
   treeView->addColumn (i18n("Shortcut"));
   treeView->setRootIsDecorated (true);
   treeView->setAllColumnsShowFocus (true);
   treeView->setSelectionMode (TQListView::Extended); 
   TQWhatsThis::add (treeView, i18n("This list contains the current phrase book in a tree structure. You can select and modify individual phrases and sub phrase books"));
   connect (treeView, TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(selectionChanged()));
   connect (treeView, TQT_SIGNAL(contextMenuRequested (TQListViewItem *, const TQPoint &, int)), this, TQT_SLOT(contextMenuRequested (TQListViewItem *, const TQPoint &, int)));
   connect (treeView, TQT_SIGNAL(dropped (TQDropEvent *, TQListViewItem *, TQListViewItem *)), this, TQT_SLOT(slotDropped (TQDropEvent *, TQListViewItem *, TQListViewItem *)));
   connect (treeView, TQT_SIGNAL(moved (TQListViewItem *, TQListViewItem *, TQListViewItem *)), this, TQT_SLOT(slotMoved (TQListViewItem *, TQListViewItem *, TQListViewItem *)));
   mainLayout->addWidget (treeView);
   
   buttonBox = new ButtonBoxWidget (page, "buttonbox");
   connect (buttonBox->lineEdit, TQT_SIGNAL(textChanged(const TQString &)), TQT_SLOT(slotTextChanged(const TQString &)));
   connect (buttonBox->noKey, TQT_SIGNAL(clicked()), TQT_SLOT(slotNoKey()));
   connect (buttonBox->customKey, TQT_SIGNAL(clicked()), TQT_SLOT(slotCustomKey()));
   connect (buttonBox->keyButton, TQT_SIGNAL(capturedShortcut(const KShortcut&)), TQT_SLOT(capturedShortcut(const KShortcut&)));
   mainLayout->addWidget (buttonBox);

   treeView->setFocus();
   selectionChanged();
}

void PhraseBookDialog::initActions() {
// The file menu
   fileNewPhrase = new KAction (i18n("&New Phrase"), "phrase_new", 0, this, TQT_SLOT(slotAddPhrase()), actionCollection(),"file_new_phrase");
   fileNewPhrase->setStatusText(i18n("Adds a new phrase"));
   fileNewPhrase->setWhatsThis (i18n("Adds a new phrase"));

   fileNewBook = new KAction (i18n("New Phrase &Book"), "phrasebook_new", 0, this, TQT_SLOT(slotAddPhrasebook()), actionCollection(),"file_new_book");
   fileNewBook->setStatusText(i18n("Adds a new phrase book into which other books and phrases can be placed"));
   fileNewBook->setWhatsThis (i18n("Adds a new phrase book into which other books and phrases can be placed"));

   fileSave = KStdAction::save(this, TQT_SLOT(slotSave()), actionCollection());
   fileSave->setStatusText(i18n("Saves the phrase book onto the hard disk"));
   fileSave->setWhatsThis (i18n("Saves the phrase book onto the hard disk"));

   fileImport = new KAction (i18n("&Import..."), "phrasebook_open", 0, this, TQT_SLOT(slotImportPhrasebook()), actionCollection(),"file_import");
   fileImport->setStatusText(i18n("Imports a file and adds its contents to the phrase book"));
   fileImport->setWhatsThis (i18n("Imports a file and adds its contents to the phrase book"));

   toolbarImport = new KToolBarPopupAction (i18n("&Import..."), "phrasebook_open", 0, this, TQT_SLOT(slotImportPhrasebook()), actionCollection(),"toolbar_import");
   toolbarImport->setStatusText(i18n("Imports a file and adds its contents to the phrase book"));
   toolbarImport->setWhatsThis (i18n("Imports a file and adds its contents to the phrase book"));

   fileImportStandardBook = new KActionMenu (i18n("I&mport Standard Phrase Book"), "phrasebook_open", actionCollection(),"file_import_standard_book");
   fileImportStandardBook->setStatusText(i18n("Imports a standard phrase book and adds its contents to the phrase book"));
   fileImportStandardBook->setWhatsThis (i18n("Imports a standard phrase book and adds its contents to the phrase book"));

   fileExport = new KAction (i18n("&Export..."), "phrasebook_save", 0, this, TQT_SLOT(slotExportPhrasebook()), actionCollection(),"file_export");
   fileExport->setStatusText(i18n("Exports the currently selected phrase(s) or phrase book(s) into a file"));
   fileExport->setWhatsThis (i18n("Exports the currently selected phrase(s) or phrase book(s) into a file"));

   filePrint = KStdAction::print(this, TQT_SLOT(slotPrint()), actionCollection());
   filePrint->setStatusText(i18n("Prints the currently selected phrase(s) or phrase book(s)"));
   filePrint->setWhatsThis (i18n("Prints the currently selected phrase(s) or phrase book(s)"));

   fileClose = KStdAction::close(this, TQT_SLOT(close()), actionCollection());
   fileClose->setStatusText(i18n("Closes the window"));
   fileClose->setWhatsThis (i18n("Closes the window"));

// The edit menu
   editCut = KStdAction::cut(this, TQT_SLOT(slotCut()), actionCollection());
   editCut->setStatusText(i18n("Cuts the currently selected entries from the phrase book and puts it to the clipboard"));
   editCut->setWhatsThis (i18n("Cuts the currently selected entries from the phrase book and puts it to the clipboard"));

   editCopy = KStdAction::copy(this, TQT_SLOT(slotCopy()), actionCollection());
   editCopy->setStatusText(i18n("Copies the currently selected entry from the phrase book to the clipboard"));
   editCopy->setWhatsThis (i18n("Copies the currently selected entry from the phrase book to the clipboard"));

   editPaste = KStdAction::paste(this, TQT_SLOT(slotPaste()), actionCollection());
   editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
   editPaste->setWhatsThis (i18n("Pastes the clipboard contents to actual position"));

   editDelete = new KAction (i18n("&Delete"), "editdelete", 0, this, TQT_SLOT(slotRemove()), actionCollection(),"edit_delete");
   editDelete->setStatusText(i18n("Deletes the selected entries from the phrase book"));
   editDelete->setWhatsThis (i18n("Deletes the selected entries from the phrase book"));

   // use the absolute path to your kmouthui.rc file for testing purpose in createGUI();
   createGUI("phrasebookdialogui.rc");
}

TQString PhraseBookDialog::displayPath (TQString filename) {
   TQFileInfo file(filename);
   TQString path = file.dirPath();
   TQString dispPath = "";
   uint position = path.find("/kmouth/books/")+TQString("/kmouth/books/").length();

   while (path.length() > position) {
      file.setFile(path);

      KDesktopFile *dirDesc = new KDesktopFile(path+"/.directory", true, "data");
      TQString name = dirDesc->readName();
      delete dirDesc;

      if (name.isNull() || name.isEmpty())
         dispPath += "/" + file.fileName ();
      else
         dispPath += "/" + name;

      path = file.dirPath();
   }
   return dispPath;
}

StandardBookList PhraseBookDialog::standardPhraseBooks() {
   TQStringList bookPaths = KGlobal::instance()->dirs()->findAllResources (
                          "data", "kmouth/books/*.phrasebook", true, true);
   TQStringList bookNames;
   TQMap<TQString,StandardBook> bookMap;
   TQStringList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      PhraseBook pbook;
      if (pbook.open (KURL( *it ))) {
         StandardBook book;
         book.name = (*pbook.begin()).getPhrase().getPhrase();
         
         book.path = displayPath(*it);
         book.filename = *it;
      
         bookNames += book.path + "/" + book.name;
         bookMap [book.path + "/" + book.name] = book;
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
   TQStringList currentNamePath = "x";
   TQPtrStack<KActionMenu> stack;
   StandardBookList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      KURL url;
      url.setPath((*it).filename);

      TQString namePath = "x/"+(*it).path;
      TQStringList dirs = TQStringList::split("/", namePath);

      TQStringList::iterator it1=currentNamePath.begin();
      TQStringList::iterator it2=dirs.begin();
      for (; (it1 != currentNamePath.end())
          && (it1 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);
      for (; it1 != currentNamePath.end(); ++it1)
         parent = stack.pop();
      for (; it2 != dirs.end(); ++it2) {
         stack.push (parent);
         KActionMenu *newParent = new KActionMenu (*it2);
         parent->insert(newParent);
         if (parent == fileImportStandardBook)
            newParent->plug(toolbarImport->popupMenu());
         parent = newParent;
      }
      currentNamePath = dirs;
      
      KAction *book = new StandardPhraseBookInsertAction (
          url, (*it).name, this, TQT_SLOT(slotImportPhrasebook (const KURL &)), actionCollection());
      parent->insert(book);
      if (parent == fileImportStandardBook)
         book->plug(toolbarImport->popupMenu());
   }
}

PhraseTreeItem *selectedItem (TQListView *treeView) {
   PhraseTreeItem *currentItem = (PhraseTreeItem *)treeView->currentItem();
   if ((currentItem == 0) || (!currentItem->isSelected()))
      return 0;

   TQListViewItemIterator it(treeView);
   while (it.current()) {
      TQListViewItem *item = it.current();
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
      buttonBox->lineEdit->setText("");
      buttonBox->lineEdit->setEnabled(false);
      buttonBox->shortcutLabel->setEnabled(false);
      buttonBox->keyButton->setShortcut("", false);
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
      TQString shortcut = currentItem->text(1);
      buttonBox->keyButton->setShortcut(currentItem->cut(), false);
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
      buttonBox->keyButton->setShortcut("", false);
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
              i18n("<qt>There are unsaved changes.<br>Do you want to apply the changes before closing the \"phrase book\" window or discard the changes?</qt>"),
          i18n("Closing \"Phrase Book\" Window"),
          KStdGuiItem::apply(), KStdGuiItem::discard(), "AutomaticSave");
      if (answer == KMessageBox::Yes) {
         slotSave();
         return true;
      }
      return (answer == KMessageBox::No);
   }
   return true;
}

void PhraseBookDialog::slotTextChanged (const TQString &s) {
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
      currentItem->setCut (KShortcut(TQString::null));
      buttonBox->keyButton->setShortcut(currentItem->cut(), false);
   }
   phrasebookChanged = true;
}

void PhraseBookDialog::slotCustomKey() {
   buttonBox->keyButton->captureShortcut();
}

void PhraseBookDialog::capturedShortcut (const KShortcut& cut) {
   if (cut.isNull()) {
      slotNoKey();
   }
   else
      setShortcut (cut);
   phrasebookChanged = true;
}

void PhraseBookDialog::setShortcut( const KShortcut& cut ) {
   // Check whether the shortcut is valid
   for (uint i = 0; i < cut.count(); i++) {
      const KKeySequence& seq = cut.seq(i);
      const KKey& key = seq.key(0);

      if (key.modFlags() == 0 && key.sym() < 0x3000
          && TQChar(key.sym()).isLetterOrNumber())
      {
         TQString s = i18n("In order to use the '%1' key as a shortcut, "
                          "it must be combined with the "
                          "Win, Alt, Ctrl, and/or Shift keys.").arg(TQChar(key.sym()));
         KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
         return;
      }
   }

   PhraseTreeItem *currentItem = selectedItem (treeView);
   // If key isn't already in use,
   if (!treeView->isKeyPresent (cut, currentItem, true)) {
      // Set new key code
      currentItem->setCut (cut);
      // Update display
      buttonBox->noKey->setChecked (false);
      buttonBox->customKey->setChecked (true);
      buttonBox->keyButton->setShortcut(currentItem->cut(), false);
   }
}

TQListViewItem *PhraseBookDialog::addBook (TQListViewItem *parent, TQListViewItem *after, PhraseBook *book) {
   TQListViewItem *newItem = treeView->addBook(parent, after, book);
   if (newItem != 0) {
      treeView->clearSelection();
      treeView->ensureItemVisible(newItem);
      treeView->setCurrentItem (newItem);
      treeView->setSelected (newItem, true);
      phrasebookChanged = true;
   }
   return newItem;
}

TQListViewItem *PhraseBookDialog::addBook (TQListViewItem *item, PhraseBook *book) {
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

void PhraseBookDialog::contextMenuRequested(TQListViewItem *, const TQPoint &pos, int) {
   TQString name;
   if (treeView->hasSelectedItems())
      name = "phrasebook_popup_sel";
   else
      name = "phrasebook_popup_nosel";

   TQPopupMenu *popup = (TQPopupMenu *)factory()->container(name,this);
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
   TQApplication::clipboard()->setData(new PhraseBookDrag(&book));
}

void PhraseBookDialog::slotPaste () {
   PhraseBook book;
   if (PhraseBookDrag::decode(TQApplication::clipboard()->data(), &book)) {
      addBook (treeView->currentItem(), &book);
   }
}

void PhraseBookDialog::slotDropped (TQDropEvent *e, TQListViewItem *parent, TQListViewItem *after) {
   PhraseBook book;
   if (PhraseBookDrag::decode(e, &book)) {
      addBook(parent, after, &book);
   }
}

void PhraseBookDialog::slotMoved (TQListViewItem *item, TQListViewItem *, TQListViewItem *) {
   treeView->ensureItemVisible(item);
   treeView->setSelected (item, true);
   phrasebookChanged = true;
}

void PhraseBookDialog::slotAddPhrasebook () {
   PhraseBook book;
   Phrase phrase(i18n("(New Phrase Book)"), "");
   book += PhraseBookEntry(phrase, 0, false);

   TQListViewItem *item = addBook (treeView->currentItem(), &book);
   item->setOpen (true);
   buttonBox->lineEdit->selectAll();
   buttonBox->lineEdit->setFocus();
}

void PhraseBookDialog::slotAddPhrase () {
   PhraseBook book;
   Phrase phrase(i18n("(New Phrase)"), "");
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
   KURL url=KFileDialog::getOpenURL(TQString::null,
        i18n("*.phrasebook|Phrase Books (*.phrasebook)\n*.txt|Plain Text Files (*.txt)\n*|All Files"), this, i18n("Import Phrasebook"));

   slotImportPhrasebook (url);
}

void PhraseBookDialog::slotImportPhrasebook (const KURL &url) {
   if(!url.isEmpty()) {
      PhraseBook book;
      if (book.open (url))
         addBook(treeView->currentItem(), &book);
      else
         KMessageBox::sorry(this,i18n("There was an error loading file\n%1").arg( url.url() ));
   }
}

void PhraseBookDialog::slotExportPhrasebook () {
   PhraseBook book;
   treeView->fillBook (&book, treeView->hasSelectedItems());
   
   KURL url;
   if (book.save (this, i18n("Export Phrase Book"), url) == -1)
      KMessageBox::sorry(this,i18n("There was an error saving file\n%1").arg( url.url() ));
}

void PhraseBookDialog::slotPrint()
{
   KPrinter printer;
   if (printer.setup(this)) {
      PhraseBook book;
      treeView->fillBook (&book, treeView->hasSelectedItems());
      
      book.print(&printer);
   }
}

#include "phrasebookdialog.moc"
