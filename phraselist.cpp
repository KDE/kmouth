/***************************************************************************
                          phraselist.cpp  -  description
                             -------------------
    begin                : Mit Sep 11 2002
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

// include files for Qt
#include <qprinter.h>
#include <qpainter.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include <qclipboard.h>

// include files for KDE
#include <klistbox.h>
#include <klineedit.h>
#include <kaudioplayer.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kmessagebox.h>

#include <stdlib.h>

// application specific includes
#include "phraselistitem.h"
#include "phraselist.h"
#include "phraseedit.h"
#include "kmouth.h"
#include "texttospeechsystem.h"
#include "phrasebook/phrasebook.h"

PhraseList::PhraseList(QWidget *parent, const char *name ) : QWidget(parent,name) {
   isInSlot = false;
   setBackgroundMode(PaletteBase);
   QVBoxLayout *layout = new QVBoxLayout (this);

   listBox = new KListBox (this);
   listBox->setFocusPolicy(QWidget::NoFocus);
   listBox->setSelectionMode (QListBox::Extended);
   QWhatsThis::add (listBox, i18n("This list contains the history of spoken sentences. You can select sentences and press the speak button for re-speaking."));
   layout->addWidget(listBox);

   QHBoxLayout *rowLayout = new QHBoxLayout ();
   layout->addLayout(rowLayout);

   lineEdit = new PhraseEdit ("", this);
   lineEdit->setFocusPolicy(QWidget::StrongFocus);
   lineEdit->setFrame(true);
   lineEdit->setEchoMode(QLineEdit::Normal);
   QWhatsThis::add (lineEdit, i18n("Into this edit field you can type a phrase. Click on the speak button in order to speak the entered phrase."));
   rowLayout->addWidget(lineEdit);

   QIconSet icon = KGlobal::iconLoader()->loadIconSet("speak", KIcon::Small);
   speakButton = new QPushButton (icon, i18n("&Speak"), this);
   speakButton->setFocusPolicy(QWidget::NoFocus);
   speakButton->setAutoDefault(false);
   QWhatsThis::add (speakButton, i18n("Speaks the currently active sentence(s). If there is some text in the edit field it is spoken. Otherwise the selected sentences in the history (if any) are spoken."));
   rowLayout->addWidget(speakButton);

   connect(listBox,  SIGNAL(selectionChanged()), SLOT(selectionChanged()));
   connect(listBox,  SIGNAL(contextMenuRequested (QListBoxItem *, const QPoint &)), SLOT(contextMenuRequested (QListBoxItem *, const QPoint &)));
   connect(lineEdit, SIGNAL(returnPressed(const QString &)), SLOT(lineEntered(const QString &)));
   connect(lineEdit, SIGNAL(textChanged  (const QString &)), SLOT(textChanged(const QString &)));
   connect(speakButton, SIGNAL( clicked ()), SLOT(speak()));
}

PhraseList::~PhraseList() {
   delete speakButton;
   delete listBox;
   delete lineEdit;
}

void PhraseList::print(KPrinter *pPrinter) {
   PhraseBook book;
   for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      book += PhraseBookEntry(Phrase(item->text()));
   }
   
   book.print (pPrinter);
}

QStringList PhraseList::getListSelection() {
   QStringList res = QStringList();

   for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      if (item->isSelected())
         res += item->text();
   }
   
   return res;
}

bool PhraseList::existListSelection() {
   for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      if (item->isSelected())
         return true;
   }

   return false;
}

bool PhraseList::existEditSelection() {
   return lineEdit->hasSelectedText();
}

void PhraseList::enableMenuEntries() {
   bool deselected = false;
   bool selected = false;
   for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      if (item->isSelected())
         selected = true;
      else
         deselected = true;
   }
   KMouthApp *theApp=(KMouthApp *) parentWidget();
   theApp->enableMenuEntries (selected, deselected);
}

void PhraseList::selectAllEntries () {
   listBox->selectAll (true);
}
void PhraseList::deselectAllEntries () {
   listBox->selectAll (false);
}

void PhraseList::speak () {
   QString phrase = lineEdit->text();
   if (phrase.isNull() || phrase.isEmpty())
      speakListSelection();
   else {
      insertIntoPhraseList (phrase, true);
      speakPhrase (phrase);
   }
}

void PhraseList::cut() {
   if (lineEdit->hasSelectedText())
      lineEdit->cut();
   else
      cutListSelection();
}

void PhraseList::copy() {
   if (lineEdit->hasSelectedText())
      lineEdit->copy();
   else
      copyListSelection();
}

void PhraseList::paste() {
   lineEdit->paste();
}

void PhraseList::insert (const QString &s) {
   setEditLineText(s);
}

void PhraseList::speakListSelection () {
   speakPhrase(getListSelection().join ("\n"));
}

void PhraseList::removeListSelection () {
   QListBoxItem *next;
   for (QListBoxItem *item = listBox->firstItem(); item != 0; item = next) {
      next = item->next();
      
      if (item->isSelected()) {
         listBox->removeItem(listBox->index(item));
      }
   }
   enableMenuEntries ();
}

void PhraseList::cutListSelection () {
   copyListSelection ();
   removeListSelection ();
}

void PhraseList::copyListSelection () {
   QApplication::clipboard()->setText (getListSelection().join ("\n"));
}

void PhraseList::lineEntered (const QString &phrase) {
   if (phrase.isNull() || phrase.isEmpty())
      speakListSelection();
   else {
      insertIntoPhraseList (phrase, true);
      speakPhrase (phrase);
   }
}

void PhraseList::speakPhrase (const QString &phrase) {
   QApplication::setOverrideCursor (KCursor::WaitCursor, false);
   KMouthApp *theApp=(KMouthApp *) parentWidget();
   theApp->getTTSSystem()->speak(phrase);
   QApplication::restoreOverrideCursor ();
}

void PhraseList::insertIntoPhraseList (const QString &phrase, bool clearEditLine) {
   int lastLine = listBox->count() - 1;
   if ((lastLine == -1) || (phrase != listBox->text(lastLine)))
      listBox->insertItem(new PhraseListItem(phrase));

   if (clearEditLine) {
      lineEdit->selectAll();
      line = "";
   }
   enableMenuEntries ();
}

void PhraseList::contextMenuRequested (QListBoxItem *, const QPoint &pos) {
   QString name;
   if (existListSelection())
      name = "phraselist_selection_popup";
   else
      name = "phraselist_popup";
   
   KMouthApp *theApp=(KMouthApp *) parentWidget();
   KXMLGUIFactory *factory = theApp->factory();
   QPopupMenu *popup = (QPopupMenu *)factory->container(name,theApp);
   if (popup != 0) {
      popup->popup(pos, 0);
   }
}

void PhraseList::textChanged (const QString &s) {
   if (!isInSlot) {
      isInSlot = true;
      line = s;
      listBox->setCurrentItem (listBox->count() - 1);
      listBox->clearSelection ();
      isInSlot = false;
   }
}

void PhraseList::selectionChanged () {
   if (!isInSlot) {
      isInSlot = true;
      
      QStringList sel = getListSelection();

      if (sel.empty())
         setEditLineText(line);
      else if (sel.count() == 1)
         setEditLineText(sel.first());
      else {
         setEditLineText("");
      }
      isInSlot = false;
   }
   enableMenuEntries ();
}

void PhraseList::setEditLineText(const QString &s) {
   lineEdit->end(false);
   while (!(lineEdit->text().isNull() || lineEdit->text().isEmpty()))
      lineEdit->backspace();
   lineEdit->insert(s);
}

void PhraseList::keyPressEvent (QKeyEvent *e) {
   if (e->key() == Qt::Key_Up) {
      bool selected = false;
      for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
         if (item->isSelected()) {
            selected = true;
         }
      }
      
      if (!selected) {
         listBox->setCurrentItem (listBox->count() - 1);
         listBox->setSelected (listBox->count() - 1, true);
      }
      else {
         int curr = listBox->currentItem();
         
         if (curr == -1) {
            isInSlot = true;
            listBox->clearSelection();
            isInSlot = false;
            curr = listBox->count() - 1;
            listBox->setCurrentItem (curr);
            listBox->setSelected (curr, true);
         }
         else if (curr != 0) {
            isInSlot = true;
            listBox->clearSelection();
            isInSlot = false;
            listBox->setCurrentItem (curr - 1);
            listBox->setSelected (curr - 1, true);
         }
      }
      
      e->accept();
   }
   else if (e->key() == Qt::Key_Down) {
      bool selected = false;
      for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
         if (item->isSelected()) {
            selected = true;
         }
      }
      
      if (selected) {
         int curr = listBox->currentItem();
      
         if (curr == (int)listBox->count() - 1) {
            listBox->clearSelection();
         }
         else if (curr != -1) {
            isInSlot = true;
            listBox->clearSelection();
            isInSlot = false;
            listBox->setCurrentItem (curr + 1);
            listBox->setSelected (curr + 1, true);
         }
      }
      e->accept();
   }
   else if ((e->state() & Qt::KeyButtonMask) == Qt::ControlButton) {
      if (e->key() == Qt::Key_C) {
         copy();
         e->accept();
      }
      else if (e->key() == Qt::Key_X) {
         cut();
         e->accept();
      }
   }
   else
      e->ignore();
}

void PhraseList::save () {
   // We want to save a history of spoken sentences here. However, as
   // the class PhraseBook does already provide a method for saving
   // phrase books in both the phrase book format and plain text file
   // format we use that method here.
   
   PhraseBook book;
   for (QListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      book += PhraseBookEntry(Phrase(item->text()));
   }

   KURL url;
   if (book.save (this, i18n("Save As"), url, false) == -1);
      KMessageBox::sorry(this,i18n("There was an error saving file\n%1").arg( url.url() ));
}

void PhraseList::open () {
   KURL url=KFileDialog::getOpenURL(QString::null,
        i18n("*.txt|Plain Text Files (*.txt)\n*.phrasebook|Phrase Books (*.phrasebook)\n*|All Files"), this, i18n("Open File as History"));

   if(!url.isEmpty())
      open (url);
}

void PhraseList::open (KURL url) {
   // We want to open a history of spoken sentences here. However, as
   // the class PhraseBook does already provide a method for opening
   // both phrase books and plain text files we use that method here.

   PhraseBook book;
   if (book.open (url)) {
      // convert PhraseBookEntryList -> QStringList
      QStringList list = book.toStringList();
      listBox->clear();
      QStringList::iterator it;
      for (it = list.begin(); it != list.end(); ++it)
         insertIntoPhraseList (*it, false);
   }
   else
      KMessageBox::sorry(this,i18n("There was an error loading file\n%1").arg( url.url() ));
}

#include "phraselist.moc"

/*
 * $Log$
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.18  2003/01/12 21:52:50  gunnar
 * Printing improved.
 *
 * Revision 1.17  2003/01/12 11:37:05  gunnar
 * Improved format list of file selectors / several small changes
 *
 * Revision 1.16  2002/12/04 16:22:02  gunnar
 * Include *.moc files
 *
 * Revision 1.15  2002/11/20 10:55:44  gunnar
 * Improved the keyboard accessibility
 *
 * Revision 1.14  2002/11/11 21:25:43  gunnar
 * Moved the parts concerning phrase books into a static library
 *
 * Revision 1.13  2002/10/28 16:58:34  gunnar
 * Import and export of phrase books implemented
 *
 * Revision 1.12  2002/10/21 16:13:30  gunnar
 * phrase book format implemented
 *
 * Revision 1.11  2002/10/15 15:53:26  gunnar
 * Improved help texts
 *
 * Revision 1.10  2002/10/08 08:20:08  gunnar
 * Small changes in the 'Whats this?' texts
 *
 * Revision 1.9  2002/10/07 17:09:33  gunnar
 * What's this? texts added
 *
 * Revision 1.8  2002/10/02 16:55:17  gunnar
 * Removed some compiler warnings and implementing the feature of opening a history at the start of KMouth
 *
 * Revision 1.7  2002/09/26 17:10:46  gunnar
 * Several small changes
 *
 * Revision 1.6  2002/09/18 09:30:40  gunnar
 * A nuber of small changes
 *
 * Revision 1.5  2002/09/13 11:51:25  gunnar
 * Added printing support
 *
 * Revision 1.4  2002/09/13 09:40:37  gunnar
 * Added support for opening a file as history
 *
 * Revision 1.3  2002/09/12 15:28:09  gunnar
 * Loading (into the edit line) and saving (the phrase list) implemented
 *
 * Revision 1.2  2002/09/11 19:03:24  gunnar
 * Implemented the popup actions
 *
 * Revision 1.1  2002/09/11 16:57:35  gunnar
 * added context menu, and moved the contents of KMouthView and KMouthDoc into a new class PhraseList
 *
 * Log: kmouthview.cpp,v
 * Revision 1.11  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 * Revision 1.10  2002/09/07 17:38:55  gunnar
 * A phrase is now only inserted into the phrase list if it is not the same as the previous one
 *
 * Revision 1.9  2002/09/07 17:29:24  gunnar
 * Moved the speak button next to the edit line
 *
 * Revision 1.8  2002/09/07 16:05:32  gunnar
 * Support for selection of multiple phrases complete
 *
 * Revision 1.7  2002/09/06 13:34:03  gunnar
 * fixed undo bug and started to develop support for selection of multiple phrases
 *
 * Revision 1.6  2002/09/02 14:36:05  gunnar
 * TODO changed
 *
 * Revision 1.5  2002/09/01 08:32:54  gunnar
 * toolbar icon and application icon added
 *
 * Revision 1.4  2002/08/29 16:38:48  gunnar
 * Added functionality for speaking the entered text
 *
 * Revision 1.3  2002/08/29 09:14:12  gunnar
 * Added functionality for scrolling through the list of spoken sentences
 *
 * Revision 1.2  2002/08/28 19:20:04  gunnar
 * Added an edit field and a list of entered lines
 *
 * Revision 1.1.1.1  2002/08/26 14:09:49  gunnar
 * New project started
 *
 */
