/***************************************************************************
                          phrasetree.h  -  description
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

#ifndef PHRASETREE_H
#define PHRASETREE_H

#include <tqpixmap.h>
#include <kshortcut.h>
#include <klistview.h>

class PhraseBook;
class PhraseShortcutRequest;

/**The class PhraseTreeItem is an ListViewItem for either a phrase or a phrase book.
  *@author Gunnar Schmi Dt
  */

class PhraseTreeItem : public KListViewItem {
   friend class PhraseTree;
private:
   /** Creates a phrase item within a sub phrase book */
   PhraseTreeItem (TQListView *parent, TQListViewItem *after, TQString phrase, KShortcut shortcut, TQPixmap icon);
   /** Creates a phrase item at the top level */
   PhraseTreeItem (TQListViewItem *parent, TQListViewItem *after, TQString phrase, KShortcut shortcut, TQPixmap icon);
   /** Creates a phrase book item within a sub phrase book */
   PhraseTreeItem (TQListView *parent, TQListViewItem *after, TQString name, TQPixmap icon);
   /** Creates a phrase book item at the top level */
   PhraseTreeItem (TQListViewItem *parent, TQListViewItem *after, TQString name, TQPixmap icon);

public:
   bool isPhrase();
   KShortcut cut();
   void setCut(KShortcut cut);

private:
   bool isPhraseValue;
   KShortcut cutValue;
};

/**
 * The class PhraseTree represents the ListView of the phrase book edit
 * dialog. It extends KListView for providing better drag-and-drop support.
 * @author Gunnar Schmi Dt
 */

class PhraseTree : public KListView  {
   friend class PhraseTreeItem;
   Q_OBJECT
  TQ_OBJECT
public: 
   PhraseTree (TQWidget *parent = 0, const char *name = 0);
   ~PhraseTree ();

   void keyPressEvent (TQKeyEvent *e);

   PhraseTreeItem *insertPhrase (TQListViewItem *parent, TQListViewItem *after, TQString phrase, TQString shortcut);
   PhraseTreeItem *insertBook (TQListViewItem *parent, TQListViewItem *after, TQString name);

   TQListViewItem *addBook (TQListViewItem *parent, TQListViewItem *after, PhraseBook *book);
   void fillBook (PhraseBook *book, bool respectSelection);

   TQDragObject *dragObject ();
   TQDragObject *dragObject (bool isDependent);

   void moveItem (TQListViewItem *item, TQListViewItem *parent, TQListViewItem *above);

   bool hasSelectedItems();
   void deleteSelectedItems();

protected:
   bool acceptDrag (TQDropEvent* event) const;

private:
   void _warning (const KKeySequence& cut, TQString sAction, TQString sTitle);
   bool isStdAccelPresent (const KShortcut& cut, bool warnUser);
   bool isGlobalKeyPresent (const KShortcut& cut, bool warnUser);
   bool isApplicationKeyPresent (const KShortcut& cut, bool warnUser);
   bool isPhraseKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser);
public:
   bool isKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser);

public slots:
   void itemExpanded (TQListViewItem *item);
   void itemCollapsed (TQListViewItem *item);

signals:
   void shortcutRequest (PhraseShortcutRequest *request);

private:
   TQPixmap phrasebook_open;
   TQPixmap phrasebook_closed;
   TQPixmap phrase;
};

#endif
