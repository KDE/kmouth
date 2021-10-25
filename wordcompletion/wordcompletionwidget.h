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

#ifndef WORDCOMPLETIONWIDGET_H
#define WORDCOMPLETIONWIDGET_H

#include "ui_wordcompletionui.h"

class QStandardItemModel;

/**
 * This class represents a configuration widget for managing dictionaries.
 * @author Gunnar Schmi Dt
 */
class WordCompletionWidget : public QWidget, public Ui::WordCompletionUI
{
    Q_OBJECT
public:
    WordCompletionWidget(QWidget *parent, const char *name);
    ~WordCompletionWidget() override;

    /**
     * This method is invoked whenever the widget should read its configuration
     * from a config file and update the user interface.
     */
    void load();

    /**
     * This function gets called when the user wants to save the settings in
     * the user interface, updating the config files.
     */
    void save();

Q_SIGNALS:
    void changed(bool);

private Q_SLOTS:
    void addDictionary();
    void deleteDictionary();
    void moveUp();
    void moveDown();
    void exportDictionary();

    void selectionChanged();
    void nameChanged(const QString &text);
    void languageSelected();

    /**
     * This slot is used to emit the signal changed when any widget changes
     * the configuration
     */
    void configChanged()
    {
        Q_EMIT changed(true);
    }

private:
    QStringList newDictionaryFiles;
    QStringList removedDictionaryFiles;
    QStandardItemModel *model;
};

#endif
