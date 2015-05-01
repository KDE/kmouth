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

#include <QWizardPage>

#include "phrasebook.h"

class QStandardItem;
class QStandardItemModel;
class KActionMenu;
class KToolBarPopupAction;

/**
 * This class represents a widget for configuring the initial phrasebook.
 * @author Gunnar Schmi Dt
 */
class InitialPhraseBookWidget : public QWizardPage
{
    Q_OBJECT
public:
    InitialPhraseBookWidget(QWidget *parent, const char *name);
    ~InitialPhraseBookWidget();

    void createBook();

private slots:
    void slotItemChanged(QStandardItem *item);

private:
    /** initializes the list of standard phrase books */
    void initStandardPhraseBooks();

    void addChildrenToBook(PhraseBook &book, QStandardItem *item);

    QStandardItemModel *m_model;
};

#endif
