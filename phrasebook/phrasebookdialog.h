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

#ifndef PHRASEBOOKDIALOG_H
#define PHRASEBOOKDIALOG_H

#include <QDomNode>
#include <QStandardItemModel>

#include <QAction>
#include <QIcon>
#include <QUrl>
#include <KXmlGuiWindow>

//#include "phrasebook.h"
#include "ui_phrasebookdialog.h"

class KActionMenu;
class KToolBarPopupAction;

/**
 * The class StandardPhraseBookInsertAction implements an Action for
 * inserting a standard phrase book.
 * @author Gunnar Schmi Dt
 */
class StandardPhraseBookInsertAction : public QAction
{
    Q_OBJECT
public:
    StandardPhraseBookInsertAction(const QUrl &url, const QString& name, const QObject* receiver, const char* slot, KActionCollection* parent);
    ~StandardPhraseBookInsertAction();

public slots:
    void slotActivated();

signals:
    void slotActivated(const QUrl &url);

private:
    QUrl url;
};

/**
 * The class PhraseBookDialog implements a dialog for editing phrase books.
 * @author Gunnar Schmi Dt
 */

class PhraseBookDialog : public KXmlGuiWindow
{
    friend class InitialPhraseBookWidget;
    Q_OBJECT
private:
    /** Constructor. It is private because this class implements the singleton
     * pattern. For creating the instance of the dialog, use the get() method.
     */
    PhraseBookDialog();

public:
    /** Returns a pointer to the instance of this dialog. As a part off the
     * singleton pattern it will make sure that there is at most one instance
     * of the dialog at a given time.
     */
    static PhraseBookDialog *get();

    /** Destructor. */
    ~PhraseBookDialog();

    bool queryClose() Q_DECL_OVERRIDE;

public slots:
    void slotTextChanged(const QString &s);
    void slotNoKey();
    void slotCustomKey();
    void slotKeySequenceChanged(const QKeySequence& sequence);

    void selectionChanged();
    void contextMenuRequested(const QPoint &pos);

    void slotRemove();
    void slotCut();
    void slotCopy();
    void slotPaste();

    void slotAddPhrasebook();
    void slotAddPhrase();

    void slotSave();
    void slotImportPhrasebook();
    void slotImportPhrasebook(const QUrl &url);
    void slotExportPhrasebook();
    //void slotPrint ();

    void slotModelChanged();

signals:
    void phrasebookConfirmed();

private:
    void initGUI();
    /** initializes the KActions of the window */
    void initActions();
    /** initializes the list of standard phrase books */
    void initStandardPhraseBooks();

    void connectEditor();
    void disconnectEditor();

    // Deserialize the book from the given QDomNode and under the given parent
    // Return the new QStandardItem so it can get focused.
    QStandardItem* deserializeBook(const QDomNode &node, QStandardItem *parent);
    // Return a serialized string of the book or phrase at the given index.
    QString serializeBook(const QModelIndex &index);

    // Get the current parent, if the current index is not a book, get it's parent.
    QModelIndex getCurrentParent();

    // Expand to, select and focus a new item from the given parameters
    void focusNewItem(QModelIndex parent, QStandardItem *item);

    void setShortcut(const QKeySequence &sequence);

    bool phrasebookChanged;

    QAction* fileNewPhrase;
    QAction* fileNewBook;
    QAction* fileSave;
    QAction* fileImport;
    KToolBarPopupAction* toolbarImport;
    KActionMenu* fileImportStandardBook;
    QAction* fileExport;
    //QAction* filePrint;
    QAction* fileClose;
    QAction* editCut;
    QAction* editCopy;
    QAction* editPaste;
    QAction* editDelete;

    QStandardItemModel* m_bookModel;
    QStandardItem *m_rootItem;

    // Keep QPrinter so settings persist
    //QPrinter *printer;

    Ui::PhraseBookDialog *m_ui;
};

#endif
