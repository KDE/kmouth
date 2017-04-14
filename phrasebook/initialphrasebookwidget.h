/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015 by Jeremy Whiting <jpwhiting@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
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
