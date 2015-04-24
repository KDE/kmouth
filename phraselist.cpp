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

// application specific includes
#include "phraselist.h"
#include "phraselistitem.h"
#include "kmouth.h"
#include "texttospeechsystem.h"
#include "phrasebook/phrasebook.h"
#include "wordcompletion/wordcompletion.h"

// include files for Qt
#include <QtGui/QPainter>
#include <QtGui/QLayout>
#include <QtGui/QApplication>
#include <QtGui/QMenu>
#include <QtGui/QClipboard>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QKeyEvent>

// include files for KDE
#include <klineedit.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kfiledialog.h>
#include <kcombobox.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>

#include <stdlib.h>


PhraseList::PhraseList(QWidget *parent, const char *name) : QWidget(parent) {
   Q_UNUSED(name);
   isInSlot = false;
   // FIXME: Remove or change PaletteBase to Qt::OpaqueMode?
   // setBackgroundMode(PaletteBase);
   QVBoxLayout *layout = new QVBoxLayout (this);

   listBox = new K3ListBox (this);
   listBox->setFocusPolicy(Qt::NoFocus);
   listBox->setSelectionMode (Q3ListBox::Extended);
   listBox->setWhatsThis( i18n("This list contains the history of spoken sentences. You can select sentences and press the speak button for re-speaking."));
   layout->addWidget(listBox);

   QHBoxLayout *rowLayout = new QHBoxLayout ();
   layout->addLayout(rowLayout);

   completion = new WordCompletion();

   dictionaryCombo = new KComboBox (this);
   configureCompletionCombo(completion->wordLists());
   rowLayout->addWidget(dictionaryCombo);

   lineEdit = new KLineEdit (QLatin1String( "" ), this);
   lineEdit->setFocusPolicy(Qt::StrongFocus);
   lineEdit->setFrame(true);
   lineEdit->setEchoMode(QLineEdit::Normal);
   lineEdit->setCompletionObject (completion);
   lineEdit->setAutoDeleteCompletionObject(true);
   lineEdit->setWhatsThis( i18n("Into this edit field you can type a phrase. Click on the speak button in order to speak the entered phrase."));
   rowLayout->addWidget(lineEdit);
   lineEdit->setFocus();

   QIcon icon = KIconLoader::global()->loadIconSet(QLatin1String( "speak" ), KIconLoader::Small);
   speakButton = new QPushButton (icon, i18n("&Speak"), this);
   speakButton->setFocusPolicy(Qt::NoFocus);
   speakButton->setAutoDefault(false);
   speakButton->setWhatsThis( i18n("Speaks the currently active sentence(s). If there is some text in the edit field it is spoken. Otherwise the selected sentences in the history (if any) are spoken."));
   rowLayout->addWidget(speakButton);

   connect(dictionaryCombo, SIGNAL (activated(QString)), completion, SLOT (setWordList(QString)));
   connect(completion, SIGNAL (wordListsChanged(QStringList)), this, SLOT (configureCompletionCombo(QStringList)));
   connect(listBox,  SIGNAL(selectionChanged()), SLOT(selectionChanged()));
   connect(listBox,  SIGNAL(contextMenuRequested(Q3ListBoxItem*,QPoint)), SLOT(contextMenuRequested(Q3ListBoxItem*,QPoint)));
   connect(lineEdit, SIGNAL(returnPressed(QString)), SLOT(lineEntered(QString)));
   connect(lineEdit, SIGNAL(textChanged(QString)), SLOT(textChanged(QString)));
   connect(speakButton, SIGNAL(clicked()), SLOT(speak()));
}

PhraseList::~PhraseList() {
   delete speakButton;
   delete listBox;
   delete lineEdit;
}

void PhraseList::print(QPrinter *pPrinter) {
   PhraseBook book;
   for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      book += PhraseBookEntry(Phrase(item->text()));
   }

   book.print (pPrinter);
}

QStringList PhraseList::getListSelection() {
   QStringList res = QStringList();

   for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      if (item->isSelected())
         res += item->text();
   }

   return res;
}

bool PhraseList::existListSelection() {
   for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
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
   for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      if (item->isSelected())
         selected = true;
      else
         deselected = true;
   }
   KMouthApp *theApp=(KMouthApp *) parentWidget();
   theApp->enableMenuEntries (selected, deselected);
}

void PhraseList::configureCompletion() {
   completion->configure();
}

void PhraseList::configureCompletionCombo(const QStringList &list) {
   QString current = completion->currentWordList();
   dictionaryCombo->clear();
   if (list.isEmpty())
      dictionaryCombo->hide();
   else if (list.count() == 1) {
      dictionaryCombo->addItems (list);
      dictionaryCombo->setCurrentIndex (0);
      dictionaryCombo->hide();
   }
   else {
      dictionaryCombo->addItems (list);
      dictionaryCombo->show();

      QStringList::ConstIterator it;
      int i = 0;
      for (it = list.begin(), i = 0; it != list.end(); ++it, ++i) {
         if (current == *it) {
            dictionaryCombo->setCurrentIndex (i);
            return;
         }
      }
   }
}

void PhraseList::saveCompletionOptions(KConfig *config) {
   KConfigGroup cg(config,"General Options");
   cg.writeEntry("Show speak button", speakButton->isVisible() || !lineEdit->isVisible());

   KConfigGroup cg2(config,"Completion");
   cg2.writeEntry("Mode", static_cast<int>(lineEdit->completionMode()));
   cg2.writeEntry("List", completion->currentWordList());
}

void PhraseList::readCompletionOptions(KConfig *config) {
   KConfigGroup cg(config,"General Options");
   if (!cg.readEntry("Show speak button", true))
      speakButton->hide();

   if (config->hasGroup ("Completion")) {
      KConfigGroup cg2(config, "Completion");
      int mode = cg2.readEntry ("Mode", int(KGlobalSettings::completionMode()));
      lineEdit->setCompletionMode (static_cast<KGlobalSettings::Completion>(mode));

      QString current = cg2.readEntry ("List", QString());
      const QStringList list = completion->wordLists();
      QStringList::ConstIterator it;
      int i = 0;
      for (it = list.constBegin(), i = 0; it != list.constEnd(); ++it, ++i) {
         if (current == *it) {
            dictionaryCombo->setCurrentIndex (i);
            return;
         }
      }
   }
}

void PhraseList::saveWordCompletion () {
   completion->save();
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
   speakPhrase(getListSelection().join (QLatin1String( "\n" )));
}

void PhraseList::removeListSelection () {
   Q3ListBoxItem *next;
   for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = next) {
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
   QApplication::clipboard()->setText (getListSelection().join (QLatin1String( "\n" )));
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
   // QApplication::setOverrideCursor (KCursor::WaitCursor, false);
   QApplication::setOverrideCursor (Qt::WaitCursor);
   KMouthApp *theApp=(KMouthApp *) parentWidget();
   QString language = completion->languageOfWordList (completion->currentWordList());
   theApp->getTTSSystem()->speak(phrase, language);
   QApplication::restoreOverrideCursor ();
}

void PhraseList::insertIntoPhraseList (const QString &phrase, bool clearEditLine) {
   int lastLine = listBox->count() - 1;
   if ((lastLine == -1) || (phrase != listBox->text(lastLine))) {
      listBox->insertItem(new PhraseListItem(phrase));
      if (clearEditLine)
         completion->addSentence (phrase);
   }

   if (clearEditLine) {
      lineEdit->selectAll();
      line.clear();
   }
   enableMenuEntries ();
}

void PhraseList::contextMenuRequested (Q3ListBoxItem *, const QPoint &pos) {
   QString name;
   if (existListSelection())
      name = QLatin1String( "phraselist_selection_popup" );
   else
      name = QLatin1String( "phraselist_popup" );

   KMouthApp *theApp=(KMouthApp *) parentWidget();
   KXMLGUIFactory *factory = theApp->factory();
   QMenu *popup = (QMenu *)factory->container(name,theApp);
   if (popup != 0) {
      popup->exec(pos, 0);
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
         setEditLineText(QLatin1String( "" ));
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
      for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
         if (item->isSelected()) {
            selected = true;
         }
      }

      if (!selected) {
         listBox->setCurrentItem (listBox->count() - 1);
         listBox->setSelected (listBox->count() - 1, true);
         listBox->ensureCurrentVisible ();
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
            listBox->ensureCurrentVisible ();
         }
         else if (curr != 0) {
            isInSlot = true;
            listBox->clearSelection();
            isInSlot = false;
            listBox->setCurrentItem (curr - 1);
            listBox->setSelected (curr - 1, true);
            listBox->ensureCurrentVisible ();
         }
      }

      e->accept();
   }
   else if (e->key() == Qt::Key_Down) {
      bool selected = false;
      for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
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
            listBox->ensureCurrentVisible ();
         }
      }
      e->accept();
   }
   else if (e->modifiers() & Qt::ControlModifier) {
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
   for (Q3ListBoxItem *item = listBox->firstItem(); item != 0; item = item->next()) {
      book += PhraseBookEntry(Phrase(item->text()));
   }

   KUrl url;
   if (book.save (this, i18n("Save As"), url, false) == -1)
      KMessageBox::sorry(this,i18n("There was an error saving file\n%1", url.url() ));
}

void PhraseList::open () {
   KUrl url=KFileDialog::getOpenUrl(KUrl(),
        i18n("*|All Files\n*.phrasebook|Phrase Books (*.phrasebook)\n*.txt|Plain Text Files (*.txt)"), this, i18n("Open File as History"));

   if(!url.isEmpty())
      open (url);
}

void PhraseList::open (KUrl url) {
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
      KMessageBox::sorry(this,i18n("There was an error loading file\n%1", url.url() ));
}

#include "phraselist.moc"
