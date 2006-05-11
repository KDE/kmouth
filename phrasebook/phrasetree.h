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

#include <QPixmap>
//Added by qt3to4:
#include <QKeyEvent>
#include <QDropEvent>
#include <kshortcut.h>
#include <k3listview.h>

class PhraseBook;
class PhraseShortcutRequest;

/**The class PhraseTreeItem is an ListViewItem for either a phrase or a phrase book.
  *@author Gunnar Schmi Dt
  */

class PhraseTreeItem : public K3ListViewItem {
   friend class PhraseTree;
private:
   /** Creates a phrase item within a sub phrase book */
   PhraseTreeItem (Q3ListView *parent, Q3ListViewItem *after, QString phrase, KShortcut shortcut, QPixmap icon);
   /** Creates a phrase item at the top level */
   PhraseTreeItem (Q3ListViewItem *parent, Q3ListViewItem *after, QString phrase, KShortcut shortcut, QPixmap icon);
   /** Creates a phrase book item within a sub phrase book */
   PhraseTreeItem (Q3ListView *parent, Q3ListViewItem *after, QString name, QPixmap icon);
   /** Creates a phrase book item at the top level */
   PhraseTreeItem (Q3ListViewItem *parent, Q3ListViewItem *after, QString name, QPixmap icon);

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
 * dialog. It extends K3ListView for providing better drag-and-drop support.
 * @author Gunnar Schmi Dt
 */

class PhraseTree : public K3ListView  {
   friend class PhraseTreeItem;
   Q_OBJECT
public: 
   PhraseTree (QWidget *parent = 0, const char *name = 0);
   ~PhraseTree ();

   void keyPressEvent (QKeyEvent *e);

   PhraseTreeItem *insertPhrase (Q3ListViewItem *parent, Q3ListViewItem *after, QString phrase, QString shortcut);
   PhraseTreeItem *insertBook (Q3ListViewItem *parent, Q3ListViewItem *after, QString name);

   Q3ListViewItem *addBook (Q3ListViewItem *parent, Q3ListViewItem *after, PhraseBook *book);
   void fillBook (PhraseBook *book, bool respectSelection);

   Q3DragObject *dragObject ();
   Q3DragObject *dragObject (bool isDependent);

   void moveItem (Q3ListViewItem *item, Q3ListViewItem *parent, Q3ListViewItem *above);

   bool hasSelectedItems();
   void deleteSelectedItems();

protected:
   bool acceptDrag (QDropEvent* event) const;

private:
   void _warning (const QKeySequence& cut, QString sAction, const QString &sTitle);
   bool isStdAccelPresent (const KShortcut& cut, bool warnUser);
   bool isGlobalKeyPresent (const KShortcut& cut, bool warnUser);
   bool isApplicationKeyPresent (const KShortcut& cut, bool warnUser);
   bool isPhraseKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser);
public:
   bool isKeyPresent (const KShortcut& cut, PhraseTreeItem* cutItem, bool warnUser);

public slots:
   void itemExpanded (Q3ListViewItem *item);
   void itemCollapsed (Q3ListViewItem *item);

signals:
   void shortcutRequest (PhraseShortcutRequest *request);

private:
   QPixmap phrasebook_open;
   QPixmap phrasebook_closed;
   QPixmap phrase;
};

#endif
