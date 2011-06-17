/***************************************************************************
                          phrasetree.cpp  -  description
                             -------------------
    begin                : Don Okt 24 2002
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

#include <klocale.h>
#include <kconfig.h>
#include <kaction.h>
#include <kstdaccel.h>
#include <kshortcutlist.h>
#include <kactionshortcutlist.h>
#include <kiconloader.h>
#include <kmessagebox.h>

#include "phrasetree.h"
#include "phrasebookdialog.h"
#include "phrasebook.h"

PhraseTreeItem::PhraseTreeItem (TQListView *tqparent, TQListViewItem *after, TQString phrase, KShortcut shortcut, TQPixmap icon)
   : KListViewItem (tqparent, after, phrase)
{
   isPhraseValue = true;
   cutValue = shortcut;
   setText(1, cutValue.toString());
   setPixmap(0, icon);
   setExpandable (false);
}

PhraseTreeItem::PhraseTreeItem (TQListViewItem *tqparent, TQListViewItem *after, TQString phrase, KShortcut shortcut, TQPixmap icon)
   : KListViewItem (tqparent, after, phrase)
{
   isPhraseValue = true;
   cutValue = shortcut;
   setText(1, cutValue.toString());
   setPixmap(0, icon);
   setExpandable (false);
}
PhraseTreeItem::PhraseTreeItem (TQListView *tqparent, TQListViewItem *after, TQString name, TQPixmap icon)
   : KListViewItem (tqparent, after, name)
{
   isPhraseValue = false;
   setPixmap(0, icon);
   setExpandable (true);
}
PhraseTreeItem::PhraseTreeItem (TQListViewItem *tqparent, TQListViewItem *after, TQString name, TQPixmap icon)
   : KListViewItem (tqparent, after, name)
{
   isPhraseValue = false;
   setPixmap(0, icon);
   setExpandable (true);
}
bool PhraseTreeItem::isPhrase () {
   return isPhraseValue;
}
KShortcut PhraseTreeItem::cut () {
   return cutValue;
}
void PhraseTreeItem::setCut (KShortcut cut) {
   cutValue = cut;
   setText(1, cut.toString());
}

// ***************************************************************************

PhraseTree::PhraseTree (TQWidget *tqparent, const char *name)
   : KListView (tqparent, name)
{
   phrasebook_open   = KGlobal::iconLoader()->loadIcon("phrasebook",        KIcon::Small);
   phrasebook_closed = KGlobal::iconLoader()->loadIcon("phrasebook_closed", KIcon::Small);
   phrase            = KGlobal::iconLoader()->loadIcon("phrase",            KIcon::Small);

   connect (this, TQT_SIGNAL(expanded (TQListViewItem *)), this, TQT_SLOT(itemExpanded (TQListViewItem *)));
   connect (this, TQT_SIGNAL(collapsed (TQListViewItem *)), this, TQT_SLOT(itemCollapsed (TQListViewItem *)));
}

PhraseTree::~PhraseTree (){
}

namespace PhraseTreePrivate {
   TQListViewItem *prevSibling (TQListViewItem *item) {
      TQListViewItem *tqparent = item->tqparent();
      TQListViewItem *above  = item->itemAbove();

      if (above == tqparent)
         return 0;

      while (above->tqparent() != tqparent)
         above = above->tqparent();

      return above;
   }

   bool findAbovePosition (TQListViewItem *item,
                           TQListViewItem **newParent,
                           TQListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      TQListViewItem *tqparent = item->tqparent();
      TQListViewItem *above  = item->itemAbove();

      if (above == 0)
         return false;
      else if (above == tqparent) {
         *newParent = tqparent->tqparent();
         *newAbove  = prevSibling (tqparent);
         return true;
      }
      else if (above->tqparent() == tqparent) {
         *newParent = tqparent;
         *newAbove  = prevSibling (above);
         return true;
      }
      else {
         while (above->tqparent()->tqparent() != tqparent)
            above = above->tqparent();
         *newParent = above->tqparent();
         *newAbove  = above;
         return true;
      }
   }

   bool findBelowPosition (TQListViewItem *item,
                           TQListViewItem **newParent,
                           TQListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      TQListViewItem *tqparent = item->tqparent();
      TQListViewItem *below  = item->nextSibling();

      if (tqparent == 0 && below == 0)
         return false;
      else if (tqparent != 0 && below == 0) {
         *newParent = tqparent->tqparent();
         *newAbove  = tqparent;
         return true;
      }
      else if (below->isOpen()) {
         *newParent = below;
         *newAbove  = 0;
         return true;
      }
      else {
         *newParent = tqparent;
         *newAbove  = below;
         return true;
      }
   }

   bool findRightPosition (TQListViewItem *item,
                           TQListViewItem **newParent,
                           TQListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      TQListViewItem *above = prevSibling (item);

      if (above == 0)
         return false;
      else if (((PhraseTreeItem *)above)->isPhrase())
         return false;
      else {
         above->setOpen(true);
         *newParent = above;
         *newAbove = 0;
         above = (*newParent)->firstChild();
         while (above != 0) {
            *newAbove = above;
            above = above->nextSibling();
         }
         return true;
      }
   }

   bool findLeftPosition (TQListViewItem *item,
                          TQListViewItem **newParent,
                          TQListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      TQListViewItem *tqparent = item->tqparent();

      if (tqparent == 0)
         return false;
      else {
         *newParent = tqparent->tqparent();
         *newAbove  = tqparent;
         return true;
      }
   }
}

void PhraseTree::moveItem (TQListViewItem *item,
                           TQListViewItem *tqparent,
                           TQListViewItem *above)
{
   if (item != 0) {
      if (item->tqparent() == 0)
         takeItem (item);
      else
         item->tqparent()->takeItem (item);

      if (tqparent == 0)
         insertItem (item);
      else
         tqparent->insertItem (item);

      item->moveItem(above);
   }
}

bool PhraseTree::hasSelectedItems() {
   TQListViewItem *i = firstChild();
   if ( !i )
       return false;
   int level = 0;
   do {
      if (i->isSelected())
         return true;

      if (i->firstChild() != 0) {
         i = i->firstChild();
         level++;
      }
      else {
         while ((i != 0) && (i->nextSibling() == 0)) {
            i = i->tqparent();
            level--;
         }
         if (i != 0)
            i = i->nextSibling();
      }
   }
   while (i != 0);

   return false;
}

void PhraseTree::deleteSelectedItems() {
   TQListViewItem *i = firstChild();
   if ( !i )
       return;
   TQListViewItem *deleteItem = 0;
   do {
      if (i->isSelected())
         deleteItem = i;

      if ((i->firstChild() != 0) && (!i->isSelected())) {
         i = i->firstChild();
      }
      else {
         while ((i != 0) && (i->nextSibling() == 0)) {
            i = i->tqparent();
         }
         if (i != 0)
            i = i->nextSibling();
      }
      if (deleteItem != 0) {
         delete deleteItem;
         deleteItem = 0;
      }
   }
   while (i != 0);
}

void PhraseTree::keyPressEvent (TQKeyEvent *e) {
   if ((e->state() & TQt::KeyButtonMask) == TQt::AltButton) {
      if (e->key() == TQt::Key_Up) {
         TQListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            TQListViewItem *tqparent;
            TQListViewItem *above;

            if (PhraseTreePrivate::findAbovePosition (item, &tqparent, &above)) {
               moveItem(item, tqparent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
      else if (e->key() == TQt::Key_Down) {
         TQListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            TQListViewItem *tqparent;
            TQListViewItem *above;

            if (PhraseTreePrivate::findBelowPosition (item, &tqparent, &above)) {
               moveItem(item, tqparent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
      else if (e->key() == TQt::Key_Left) {
         TQListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            TQListViewItem *tqparent;
            TQListViewItem *above;

            if (PhraseTreePrivate::findLeftPosition (item, &tqparent, &above)) {
               moveItem(item, tqparent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
      else if (e->key() == TQt::Key_Right) {
         TQListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            TQListViewItem *tqparent;
            TQListViewItem *above;

            if (PhraseTreePrivate::findRightPosition (item, &tqparent, &above)) {
               moveItem(item, tqparent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
   }
   KListView::keyPressEvent(e);
}

PhraseTreeItem *PhraseTree::insertPhrase (TQListViewItem *tqparent, TQListViewItem *after, TQString phrase, TQString shortcut) {
   KShortcut cut = KShortcut(shortcut);
   if (isKeyPresent (cut, 0, false))
      cut = KShortcut(TQString());

   if (tqparent == 0)
      return new PhraseTreeItem (this, after, phrase, cut, this->phrase);
   else
      return new PhraseTreeItem (tqparent, after, phrase, cut, this->phrase);
}

PhraseTreeItem *PhraseTree::insertBook (TQListViewItem *tqparent, TQListViewItem *after, TQString name) {
   if (tqparent == 0)
      return new PhraseTreeItem (this, after, name, phrasebook_closed);
   else
      return new PhraseTreeItem (tqparent, after, name, phrasebook_closed);
}

TQListViewItem *PhraseTree::addBook (TQListViewItem *tqparent, TQListViewItem *after, PhraseBook *book) {
   TQListViewItem *last = after;
   int level = 0;
   PhraseBookEntryList::iterator it;
   for (it = book->begin(); it != book->end(); ++it) {
      int newLevel = (*it).getLevel();
      while (level < newLevel) {
         tqparent = insertBook(tqparent, last, "");
         last = 0;
         level++;
      }
      while (level > newLevel) {
         last = tqparent;
         if (tqparent != 0)
            tqparent = tqparent->tqparent();
         level--;
      }

      if ((*it).isPhrase()) {
         Phrase phrase = (*it).getPhrase();
         last = insertPhrase (tqparent, last, phrase.getPhrase(), phrase.getShortcut());
      }
      else {
         Phrase phrase = (*it).getPhrase();
         tqparent = insertBook(tqparent, last, phrase.getPhrase());
         last = 0;
         level++;
      }
   }
   while (level > 0) {
      last = tqparent;
      if (tqparent != 0)
         tqparent = tqparent->tqparent();
      level--;
   }
   return last;
}

void PhraseTree::fillBook (PhraseBook *book, bool respectSelection) {
   TQListViewItem *i = firstChild();
   int level = 0;
   if ( !i )
       return;
   do {
      if (i->isSelected() || !respectSelection || level > 0) {
         PhraseTreeItem *it = (PhraseTreeItem *)i;
         Phrase phrase(it->text(0), it->cut().toStringInternal());
         *book += PhraseBookEntry(phrase, level, it->isPhrase());
      }

      if (i->firstChild() != 0) {
         if (i->isSelected() || !respectSelection || level > 0)
            level++;
         i = i->firstChild();
      }
      else {
         while ((i != 0) && (i->nextSibling() == 0)) {
            i = i->tqparent();
            if (level > 0)
               level--;
         }
         if (i != 0)
            i = i->nextSibling();
      }
   }
   while (i != 0);
}

TQDragObject *PhraseTree::dragObject () {
   return dragObject (true);
}

TQDragObject *PhraseTree::dragObject (bool isDependent) {
   PhraseBook book;
   fillBook (&book, true);
   if (isDependent)
      return new PhraseBookDrag(&book, viewport());
   return new PhraseBookDrag(&book);
}

bool PhraseTree::acceptDrag (TQDropEvent* event) const {
   if (KListView::acceptDrag (event))
      return true;
   else
      return PhraseBookDrag::canDecode(event);
}

// Returns iSeq index if cut2 has a sequence of equal or higher priority
// to a sequence in cut, else -1
static int keyConflict (const KShortcut& cut, const KShortcut& cut2) {
   for (uint iSeq = 0; iSeq < cut.count(); iSeq++) {
      for (uint iSeq2 = 0; iSeq2 <= iSeq && iSeq2 < cut2.count(); iSeq2++) {
         if (cut.seq(iSeq) == cut2.seq(iSeq2))
            return iSeq;
       }
   }
   return -1;
}

void PhraseTree::_warning (const KKeySequence& cut, TQString sAction, TQString sTitle) {
   sAction = sAction.stripWhiteSpace();

   TQString s =
       i18n("The '%1' key combination has already been allocated "
       "to %2.\n"
       "Please choose a unique key combination.").
       tqarg(cut.toString()).tqarg(sAction);

   KMessageBox::sorry( this, s, sTitle );
}

bool PhraseTree::isStdAccelPresent (const KShortcut& cut, bool warnUser) {
   for (uint iSeq = 0; iSeq < cut.count(); iSeq++) {
      const KKeySequence& seq = cut.seq(iSeq);

      KStdAccel::StdAccel id = KStdAccel::findStdAccel( seq );
      if( id != KStdAccel::AccelNone
          && keyConflict (cut, KStdAccel::shortcut(id)) > -1)
      {
         if (warnUser)
            _warning (cut.seq(iSeq),
                      i18n("the standard \"%1\" action").tqarg(KStdAccel::label(id)),
                      i18n("Conflict with Standard Application Shortcut"));
         return true;
      }
   }
   return false;
}

bool PhraseTree::isGlobalKeyPresent (const KShortcut& cut, bool warnUser) {
   TQMap<TQString, TQString> mapEntry = KGlobal::config()->entryMap ("Global Shortcuts");
   TQMap<TQString, TQString>::Iterator it;
   for (it = mapEntry.begin(); it != mapEntry.end(); ++it) {
      int iSeq = keyConflict (cut, KShortcut(*it));
      if (iSeq > -1) {
         if (warnUser)
            _warning (cut.seq(iSeq),
                      i18n("the global \"%1\" action").tqarg(it.key()),
                      i18n("Conflict with Global Shortcuts"));
         return true;
      }
   }
   return false;
}

bool PhraseTree::isPhraseKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser) {
   for (TQListViewItemIterator it(this); it.current(); ++it) {
      PhraseTreeItem* item = dynamic_cast<PhraseTreeItem*>(it.current());
      if ((item != 0) && (item != cutItem)) {
         int iSeq = keyConflict (cut, item->cut());
         if (iSeq > -1) {
            if (warnUser)
               _warning (cut.seq(iSeq),
                         i18n("an other phrase"),
                         i18n("Key Conflict"));
            return true;
         }
      }
   }
   return false;
}

bool PhraseTree::isKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser) {
   if (isStdAccelPresent (cut, warnUser))
      return true;

   if (isGlobalKeyPresent (cut, warnUser))
      return true;

   if (isPhraseKeyPresent (cut, cutItem, warnUser))
      return true;

   return false;
}

void PhraseTree::itemExpanded (TQListViewItem *item) {
   PhraseTreeItem *i = (PhraseTreeItem *)item;
   if (!i->isPhrase())
      i->setPixmap(0, phrasebook_open);
}

void PhraseTree::itemCollapsed (TQListViewItem *item) {
   PhraseTreeItem *i = (PhraseTreeItem *)item;
   if (!i->isPhrase())
      i->setPixmap(0, phrasebook_closed);
}

// ***************************************************************************

#include "phrasetree.moc"
