/***************************************************************************
                          initialphrasebookwidget.cpp  -  description
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

#include "initialphrasebookwidget.h"

// include files for Qt
#include <QLabel>
#include <QStack>
#include <QVBoxLayout>

#include <QDebug>

// include files for KDE
#include <k3listview.h>
#include <kactionmenu.h>
#include <kdesktopfile.h>
#include <kfiledialog.h>
#include <klocalizedstring.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <ktoolbarpopupaction.h>
#include <kxmlguifactory.h>

#include "phrasebook.h"

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

   if (((lv != 0) && (!lv->isEnabled())) || (!isEnabled()))
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
   : QWizardPage(parent)
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
   StandardBookList bookPaths = PhraseBook::standardPhraseBooks();

   Q3ListViewItem *parent = 0;
   Q3ListViewItem *last = 0;
   QStringList currentNamePath;
   currentNamePath<<QLatin1String("");
   QStack<Q3ListViewItem *> stack;
   StandardBookList::iterator it;
   for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
      QString namePath = (*it).path;
      QStringList dirs = namePath.split( QLatin1Char( '/' ));

      QStringList::iterator it1=currentNamePath.begin();
      QStringList::iterator it2=dirs.begin();
      for (; (it1 != currentNamePath.end())
          && (it2 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);

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
