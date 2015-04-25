/***************************************************************************
                          initialphrasebookwidget.h  -  description
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

#ifndef INITIALPHRASEBOOKWIDGET_H
#define INITIALPHRASEBOOKWIDGET_H

#include <Qt3Support/Q3ListView>
#include <QWizardPage>

#include "phrasebook.h"

class K3ListView;
class KActionMenu;
class KToolBarPopupAction;

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

#endif
