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

#ifndef PHRASELIST_H
#define PHRASELIST_H

// include files for KDE
#include <QUrl>

// include files for Qt
#include <QWidget>

class KComboBox;
class KLineEdit;
class QKeyEvent;
class QListView;
class QPrinter;
class QPushButton;
class QStandardItemModel;
class WordCompletion;

/**
 * This class represents a phrase list. It contains methods for manipulating
 * the phraselist and also methods for viewing the list.
 * The phrase list consists of an edit field for entering phrases and a list
 * box for the spoken phrases.
 *
 * @author Gunnar Schmi Dt
 */

class PhraseList : public QWidget
{
    Q_OBJECT
public:
    explicit PhraseList(QWidget *parent = nullptr, const QString &name = QString());
    ~PhraseList() override;

    /** contains the implementation for printing functionality */
    void print(QPrinter *pPrinter);

    QStringList getListSelection();

    bool existListSelection();
    bool existEditSelection();

public Q_SLOTS:
    /** Called whenever the user wants the contents of the edit line to be spoken. */
    void speak();

    void stopSpeaking();

    void pauseResume();

    void cut();
    void copy();
    void paste();

    /** Insert s into the edit field. */
    void insert(const QString &s);

    /** Called whenever the user wants the selected list entries to be spoken. */
    void speakListSelection();

    void removeListSelection();
    void cutListSelection();
    void copyListSelection();

    void save();
    void open();
    void open(const QUrl &url);

    void selectAllEntries();
    void deselectAllEntries();

    void configureCompletion();
    void saveWordCompletion();
    void saveCompletionOptions();
    void readCompletionOptions();

protected Q_SLOTS:
    void lineEntered(const QString &phrase);
    void contextMenuRequested(const QPoint &pos);
    void textChanged(const QString &s);
    void selectionChanged();
    void keyPressEvent(QKeyEvent *e) override;
    void configureCompletionCombo(const QStringList &list);

private:
    QListView *m_listView;
    QStandardItemModel *m_model;
    KComboBox *dictionaryCombo;
    KLineEdit *lineEdit;
    QPushButton *speakButton;
    QString line;
    WordCompletion *completion;

    bool isInSlot;

    void speakPhrase(const QString &phrase);
    void setEditLineText(const QString &s);
    void insertIntoPhraseList(const QString &phrase, bool clearEditLine);

    void enableMenuEntries();
};

#endif
