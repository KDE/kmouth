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

PhraseTreeItem::PhraseTreeItem (QListView *parent, QListViewItem *after, QString phrase, KShortcut shortcut, QPixmap icon)
   : KListViewItem (parent, after, phrase)
{
   isPhraseValue = true;
   cutValue = shortcut;
   setText(1, cutValue.toString());
   setPixmap(0, icon);
   setExpandable (false);
}

PhraseTreeItem::PhraseTreeItem (QListViewItem *parent, QListViewItem *after, QString phrase, KShortcut shortcut, QPixmap icon)
   : KListViewItem (parent, after, phrase)
{
   isPhraseValue = true;
   cutValue = shortcut;
   setText(1, cutValue.toString());
   setPixmap(0, icon);
   setExpandable (false);
}
PhraseTreeItem::PhraseTreeItem (QListView *parent, QListViewItem *after, QString name, QPixmap icon)
   : KListViewItem (parent, after, name)
{
   isPhraseValue = false;
   setPixmap(0, icon);
   setExpandable (true);
}
PhraseTreeItem::PhraseTreeItem (QListViewItem *parent, QListViewItem *after, QString name, QPixmap icon)
   : KListViewItem (parent, after, name)
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

PhraseTree::PhraseTree (QWidget *parent, const char *name)
   : KListView (parent, name)
{
   phrasebook_open   = KGlobal::iconLoader()->loadIcon("phrasebook",        KIcon::Small);
   phrasebook_closed = KGlobal::iconLoader()->loadIcon("phrasebook_closed", KIcon::Small);
   phrase            = KGlobal::iconLoader()->loadIcon("phrase",            KIcon::Small);

   connect (this, SIGNAL(expanded (QListViewItem *)), this, SLOT(itemExpanded (QListViewItem *)));
   connect (this, SIGNAL(collapsed (QListViewItem *)), this, SLOT(itemCollapsed (QListViewItem *)));
}

PhraseTree::~PhraseTree (){
}

namespace PhraseTreePrivate {
   QListViewItem *prevSibling (QListViewItem *item) {
      QListViewItem *parent = item->parent();
      QListViewItem *above  = item->itemAbove();

      if (above == parent)
         return 0;

      while (above->parent() != parent)
         above = above->parent();

      return above;
   }

   bool findAbovePosition (QListViewItem *item,
                           QListViewItem **newParent,
                           QListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      QListViewItem *parent = item->parent();
      QListViewItem *above  = item->itemAbove();

      if (above == 0)
         return false;
      else if (above == parent) {
         *newParent = parent->parent();
         *newAbove  = prevSibling (parent);
         return true;
      }
      else if (above->parent() == parent) {
         *newParent = parent;
         *newAbove  = prevSibling (above);
         return true;
      }
      else {
         while (above->parent()->parent() != parent)
            above = above->parent();
         *newParent = above->parent();
         *newAbove  = above;
         return true;
      }
   }

   bool findBelowPosition (QListViewItem *item,
                           QListViewItem **newParent,
                           QListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      QListViewItem *parent = item->parent();
      QListViewItem *below  = item->nextSibling();

      if (parent == 0 && below == 0)
         return false;
      else if (parent != 0 && below == 0) {
         *newParent = parent->parent();
         *newAbove  = parent;
         return true;
      }
      else if (below->isOpen()) {
         *newParent = below;
         *newAbove  = 0;
         return true;
      }
      else {
         *newParent = parent;
         *newAbove  = below;
         return true;
      }
   }

   bool findRightPosition (QListViewItem *item,
                           QListViewItem **newParent,
                           QListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      QListViewItem *above = prevSibling (item);

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

   bool findLeftPosition (QListViewItem *item,
                          QListViewItem **newParent,
                          QListViewItem **newAbove)
   {
      if (item == 0)
         return false;

      QListViewItem *parent = item->parent();

      if (parent == 0)
         return false;
      else {
         *newParent = parent->parent();
         *newAbove  = parent;
         return true;
      }
   }
}

void PhraseTree::moveItem (QListViewItem *item,
                           QListViewItem *parent,
                           QListViewItem *above)
{
   if (item != 0) {
      if (item->parent() == 0)
         takeItem (item);
      else
         item->parent()->takeItem (item);

      if (parent == 0)
         insertItem (item);
      else
         parent->insertItem (item);

      item->moveItem(above);
   }
}

bool PhraseTree::hasSelectedItems() {
   QListViewItem *i = firstChild();
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
            i = i->parent();
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
   QListViewItem *i = firstChild();
   if ( !i )
       return;
   QListViewItem *deleteItem = 0;
   do {
      if (i->isSelected())
         deleteItem = i;

      if ((i->firstChild() != 0) && (!i->isSelected())) {
         i = i->firstChild();
      }
      else {
         while ((i != 0) && (i->nextSibling() == 0)) {
            i = i->parent();
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

void PhraseTree::keyPressEvent (QKeyEvent *e) {
   if ((e->state() & Qt::KeyButtonMask) == Qt::AltButton) {
      if (e->key() == Qt::Key_Up) {
         QListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            QListViewItem *parent;
            QListViewItem *above;

            if (PhraseTreePrivate::findAbovePosition (item, &parent, &above)) {
               moveItem(item, parent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
      else if (e->key() == Qt::Key_Down) {
         QListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            QListViewItem *parent;
            QListViewItem *above;

            if (PhraseTreePrivate::findBelowPosition (item, &parent, &above)) {
               moveItem(item, parent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
      else if (e->key() == Qt::Key_Left) {
         QListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            QListViewItem *parent;
            QListViewItem *above;

            if (PhraseTreePrivate::findLeftPosition (item, &parent, &above)) {
               moveItem(item, parent, above);
               setCurrentItem (item);
               item->setSelected(true);
            }
         }
         e->accept();
         return;
      }
      else if (e->key() == Qt::Key_Right) {
         QListViewItem *item = currentItem();
         if ((item != 0) && (item->isSelected())) {
            QListViewItem *parent;
            QListViewItem *above;

            if (PhraseTreePrivate::findRightPosition (item, &parent, &above)) {
               moveItem(item, parent, above);
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

PhraseTreeItem *PhraseTree::insertPhrase (QListViewItem *parent, QListViewItem *after, QString phrase, QString shortcut) {
   KShortcut cut = KShortcut(shortcut);
   if (isKeyPresent (cut, 0, false))
      cut = KShortcut(QString::null);

   if (parent == 0)
      return new PhraseTreeItem (this, after, phrase, cut, this->phrase);
   else
      return new PhraseTreeItem (parent, after, phrase, cut, this->phrase);
}

PhraseTreeItem *PhraseTree::insertBook (QListViewItem *parent, QListViewItem *after, QString name) {
   if (parent == 0)
      return new PhraseTreeItem (this, after, name, phrasebook_closed);
   else
      return new PhraseTreeItem (parent, after, name, phrasebook_closed);
}

QListViewItem *PhraseTree::addBook (QListViewItem *parent, QListViewItem *after, PhraseBook *book) {
   QListViewItem *last = after;
   int level = 0;
   PhraseBookEntryList::iterator it;
   for (it = book->begin(); it != book->end(); ++it) {
      int newLevel = (*it).getLevel();
      while (level < newLevel) {
         parent = insertBook(parent, last, "");
         last = 0;
         level++;
      }
      while (level > newLevel) {
         last = parent;
         if (parent != 0)
            parent = parent->parent();
         level--;
      }

      if ((*it).isPhrase()) {
         Phrase phrase = (*it).getPhrase();
         last = insertPhrase (parent, last, phrase.getPhrase(), phrase.getShortcut());
      }
      else {
         Phrase phrase = (*it).getPhrase();
         parent = insertBook(parent, last, phrase.getPhrase());
         last = 0;
         level++;
      }
   }
   while (level > 0) {
      last = parent;
      if (parent != 0)
         parent = parent->parent();
      level--;
   }
   return last;
}

void PhraseTree::fillBook (PhraseBook *book, bool respectSelection) {
   QListViewItem *i = firstChild();
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
            i = i->parent();
            if (level > 0)
               level--;
         }
         if (i != 0)
            i = i->nextSibling();
      }
   }
   while (i != 0);
}

QDragObject *PhraseTree::dragObject () {
   return dragObject (true);
}

QDragObject *PhraseTree::dragObject (bool isDependent) {
   PhraseBook book;
   fillBook (&book, true);
   if (isDependent)
      return new PhraseBookDrag(&book, viewport());
   return new PhraseBookDrag(&book);
}

bool PhraseTree::acceptDrag (QDropEvent* event) const {
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

void PhraseTree::_warning (const KKeySequence& cut, QString sAction, QString sTitle) {
   sAction = sAction.stripWhiteSpace();

   QString s =
       i18n("The '%1' key combination has already been allocated "
       "to %2.\n"
       "Please choose a unique key combination.").
       arg(cut.toString()).arg(sAction);

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
                      i18n("the standard \"%1\" action").arg(KStdAccel::label(id)),
                      i18n("Conflict with Standard Application Shortcut"));
         return true;
      }
   }
   return false;
}

bool PhraseTree::isGlobalKeyPresent (const KShortcut& cut, bool warnUser) {
   QMap<QString, QString> mapEntry = KGlobal::config()->entryMap ("Global Shortcuts");
   QMap<QString, QString>::Iterator it;
   for (it = mapEntry.begin(); it != mapEntry.end(); ++it) {
      int iSeq = keyConflict (cut, KShortcut(*it));
      if (iSeq > -1) {
         if (warnUser)
            _warning (cut.seq(iSeq),
                      i18n("the global \"%1\" action").arg(it.key()),
                      i18n("Conflict with Global Shortcuts"));
         return true;
      }
   }
   return false;
}

bool PhraseTree::isPhraseKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser) {
   for (QListViewItemIterator it(this); it.current(); ++it) {
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

void PhraseTree::itemExpanded (QListViewItem *item) {
   PhraseTreeItem *i = (PhraseTreeItem *)item;
   if (!i->isPhrase())
      i->setPixmap(0, phrasebook_open);
}

void PhraseTree::itemCollapsed (QListViewItem *item) {
   PhraseTreeItem *i = (PhraseTreeItem *)item;
   if (!i->isPhrase())
      i->setPixmap(0, phrasebook_closed);
}

// ***************************************************************************

#include "phrasetree.moc"
