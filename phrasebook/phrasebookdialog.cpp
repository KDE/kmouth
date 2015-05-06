/***************************************************************************
                          phrasebookdialog.cpp  -  description
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

#include "phrasebookdialog.h"

// include files for Qt
#include <QClipboard>
#include <QMimeData>
#include <QStack>
#include <QStandardPaths>

// include files for KDE
#include <KActionMenu>
#include <KFileDialog>
#include <KMessageBox>
#include <KToolBarPopupAction>
#include <KXMLGUIFactory>

#include "phrasebook.h"

const int kTextColumn = 0;
const int kShortcutColumn = 1;

const QString kName = QLatin1String("name");
const QString kShortcut = QLatin1String("shortcut");
const QString kPhraseBook = QLatin1String("phrasebook");
const QString kPhrase = QLatin1String("phrase");

const QString kPhraseBookXML = QLatin1String("<phrasebook name=\"%1\">\n%2</phrasebook>\n");
const QString kPhraseXML = QLatin1String("<phrase shortcut=\"%2\">%1</phrase>\n");
const QString kWholeBookXML = QLatin1String("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
                              "<!DOCTYPE phrasebook>\n"
                              "<phrasebook>\n%1"
                              "</phrasebook>");

const QIcon kPhraseBookIcon = KIcon(kPhraseBook);
const QIcon kPhraseIcon = KIcon(kPhrase);

StandardPhraseBookInsertAction::StandardPhraseBookInsertAction(const KUrl &url, const QString& name, const QObject* receiver, const char* slot, KActionCollection* parent)
    : KAction(KIcon(QLatin1String("phrasebook")), name, parent)
{
    this->url = url;
    connect(this, SIGNAL(triggered(bool)), this, SLOT(slotActivated()));
    connect(this, SIGNAL(slotActivated(const KUrl &)), receiver, slot);
    parent->addAction(name, this);
}

StandardPhraseBookInsertAction::~StandardPhraseBookInsertAction()
{
}

void StandardPhraseBookInsertAction::slotActivated()
{
    emit slotActivated(url);
}


namespace PhraseBookPrivate
{
PhraseBookDialog *instance = 0;
}

PhraseBookDialog::PhraseBookDialog()
    : KXmlGuiWindow(0)
{
    // Create model with 2 columns and no rows
    m_bookModel = new QStandardItemModel(0, 2, this);
    m_rootItem = m_bookModel->invisibleRootItem();
    m_bookModel->setHeaderData(kTextColumn, Qt::Horizontal, i18n("Phrase"));
    m_bookModel->setHeaderData(kShortcutColumn, Qt::Horizontal, i18n("Shortcut"));

    setObjectName(QLatin1String("phraseEditDialog"));
    setCaption(i18n("Phrase Book"));
    initGUI();
    initActions();
    initStandardPhraseBooks();
    QString standardBook = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("standard.phrasebook"));
    if (!standardBook.isNull() && !standardBook.isEmpty()) {
        QFile file(standardBook);
        file.open(QIODevice::ReadOnly);
        QDomDocument document;
        document.setContent(&file);

        QDomNodeList nodes = document.documentElement().childNodes();
        for (int i = 0; i < nodes.count(); ++i) {
            deserializeBook(nodes.at(i), m_rootItem);
        }

        selectionChanged();
        phrasebookChanged = false;
        fileSave->setEnabled(false);
    }

    // Watch any changes in the model.
    connect(m_bookModel, SIGNAL(dataChanged(QModelIndex, QModelIndex)),
            this, SLOT(slotModelChanged()));
    connect(m_bookModel, SIGNAL(rowsInserted(QModelIndex, int, int)),
            this, SLOT(slotModelChanged()));
    connect(m_bookModel, SIGNAL(rowsMoved(QModelIndex, int, int, QModelIndex, int)),
            this, SLOT(slotModelChanged()));
    connect(m_bookModel, SIGNAL(rowsRemoved(QModelIndex, int, int)),
            this, SLOT(slotModelChanged()));

    //printer = 0;
    // i18n("Edit Phrase Book")
}

PhraseBookDialog *PhraseBookDialog::get()
{
    if (PhraseBookPrivate::instance == 0)
        PhraseBookPrivate::instance = new PhraseBookDialog();
    return PhraseBookPrivate::instance;
}

PhraseBookDialog::~PhraseBookDialog()
{
    PhraseBookPrivate::instance = 0;
    //delete printer;
}

QStandardItem* PhraseBookDialog::deserializeBook(const QDomNode &node, QStandardItem *parent)
{
    QString text;
    QString shortcut;
    bool isBook = (node.nodeName() == kPhraseBook);
    text = isBook ? node.attributes().namedItem(kName).nodeValue() : node.lastChild().nodeValue();
    shortcut = isBook ? QString() : node.attributes().namedItem(kShortcut).nodeValue();

    QStandardItem *item = isBook ? new QStandardItem(kPhraseBookIcon, text) : new QStandardItem(kPhraseIcon, text);
    QStandardItem *shortcutItem = new QStandardItem(shortcut);

    if (!isBook) {
        item->setDropEnabled(false);
        shortcutItem->setDropEnabled(false);
    }
    QList<QStandardItem*> items;
    items << item << shortcutItem;
    parent->appendRow(items);

    if (isBook) {
        // Iterate over the document creating QStandardItems as needed
        QDomNodeList childNodes = node.childNodes();
        for (int i = 0; i < childNodes.count(); ++i) {
            const QDomNode child = childNodes.at(i);
            deserializeBook(child, item);
        }
    }
    return item;
}

QString PhraseBookDialog::serializeBook(const QModelIndex &index)
{
    QString value;
    if (index.isValid()) {
        QStandardItem *item = m_bookModel->itemFromIndex(index);
        QStandardItem *shortcutItem = m_bookModel->itemFromIndex(index.sibling(index.row(), kShortcutColumn));
        bool isBook = item->isDropEnabled(); // We know it's a book if it's drop enabled
        if (isBook) {
            QString childrenText;
            for (int i = 0; i < item->rowCount(); ++i) {
                childrenText += serializeBook(index.child(i, 0));
            }
            value = kPhraseBookXML.arg(item->text()).arg(childrenText);
        } else {
            value = kPhraseXML.arg(item->text()).arg(shortcutItem->text());
        }
    } else {
        // Do the whole model, inside a <phrasebook> tag.
        int rows = m_bookModel->rowCount(QModelIndex());
        QString childrenText;
        for (int i = 0; i < rows; ++i) {
            childrenText += serializeBook(m_bookModel->index(i, 0));
        }
        value = kWholeBookXML.arg(childrenText);
    }
    return value;
}

void PhraseBookDialog::initGUI()
{
    QWidget *page = new QWidget(this);

    m_ui = new Ui::PhraseBookDialog();
    m_ui->setupUi(page);
    setCentralWidget(page);

    m_ui->treeView->setModel(m_bookModel);
    connect(m_ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
            this, SLOT(selectionChanged()));
    connect(m_ui->treeView, SIGNAL(customContextMenuRequested(QPoint)),
            SLOT(contextMenuRequested(QPoint)));
    connectEditor();
}

void PhraseBookDialog::slotModelChanged()
{
    phrasebookChanged = true;
    fileSave->setEnabled(true);
}

void PhraseBookDialog::initActions()
{
    // The file menu
    fileNewPhrase = actionCollection()->addAction(QLatin1String("file_new_phrase"));
    fileNewPhrase->setIcon(KIcon(QLatin1String("phrase_new")));
    fileNewPhrase->setText(i18n("&New Phrase"));
    connect(fileNewPhrase, SIGNAL(triggered(bool)), this, SLOT(slotAddPhrase()));
    fileNewPhrase->setToolTip(i18n("Adds a new phrase"));
    fileNewPhrase->setWhatsThis(i18n("Adds a new phrase"));

    fileNewBook = actionCollection()->addAction(QLatin1String("file_new_book"));
    fileNewBook->setIcon(KIcon(QLatin1String("phrasebook_new")));
    fileNewBook->setText(i18n("New Phrase &Book"));
    connect(fileNewBook, SIGNAL(triggered(bool)), this, SLOT(slotAddPhrasebook()));
    fileNewBook->setToolTip(i18n("Adds a new phrase book into which other books and phrases can be placed"));
    fileNewBook->setWhatsThis(i18n("Adds a new phrase book into which other books and phrases can be placed"));

    fileSave = KStandardAction::save(this, SLOT(slotSave()), actionCollection());
    fileSave->setToolTip(i18n("Saves the phrase book onto the hard disk"));
    fileSave->setWhatsThis(i18n("Saves the phrase book onto the hard disk"));

    fileImport = actionCollection()->addAction(QLatin1String("file_import"));
    fileImport->setIcon(KIcon(QLatin1String("phrasebook_open")));
    fileImport->setText(i18n("&Import..."));
    connect(fileImport, SIGNAL(triggered(bool)), this, SLOT(slotImportPhrasebook()));
    fileImport->setToolTip(i18n("Imports a file and adds its contents to the phrase book"));
    fileImport->setWhatsThis(i18n("Imports a file and adds its contents to the phrase book"));

    toolbarImport = new KToolBarPopupAction(KIcon(QLatin1String("phrasebook_open")), i18n("&Import..."), this);
    actionCollection()->addAction(QLatin1String("toolbar_import"), toolbarImport);
    connect(toolbarImport, SIGNAL(triggered(bool)), this, SLOT(slotImportPhrasebook()));
    toolbarImport->setToolTip(i18n("Imports a file and adds its contents to the phrase book"));
    toolbarImport->setWhatsThis(i18n("Imports a file and adds its contents to the phrase book"));

    fileImportStandardBook = actionCollection()->add<KActionMenu>(QLatin1String("file_import_standard_book"));
    fileImportStandardBook->setIcon(KIcon(QLatin1String("phrasebook_open")));
    fileImportStandardBook->setText(i18n("I&mport Standard Phrase Book"));
    fileImportStandardBook->setToolTip(i18n("Imports a standard phrase book and adds its contents to the phrase book"));
    fileImportStandardBook->setWhatsThis(i18n("Imports a standard phrase book and adds its contents to the phrase book"));

    fileExport = actionCollection()->addAction(QLatin1String("file_export"));
    fileExport->setIcon(KIcon(QLatin1String("phrasebook_save")));
    fileExport->setText(i18n("&Export..."));
    connect(fileExport, SIGNAL(triggered(bool)), this, SLOT(slotExportPhrasebook()));
    fileExport->setToolTip(i18n("Exports the currently selected phrase(s) or phrase book(s) into a file"));
    fileExport->setWhatsThis(i18n("Exports the currently selected phrase(s) or phrase book(s) into a file"));

    //filePrint = KStandardAction::print(this, SLOT(slotPrint()), actionCollection());
    //filePrint->setToolTip(i18n("Prints the currently selected phrase(s) or phrase book(s)"));
    //filePrint->setWhatsThis (i18n("Prints the currently selected phrase(s) or phrase book(s)"));

    fileClose = KStandardAction::close(this, SLOT(close()), actionCollection());
    fileClose->setToolTip(i18n("Closes the window"));
    fileClose->setWhatsThis(i18n("Closes the window"));

    // The edit menu
    editCut = KStandardAction::cut(this, SLOT(slotCut()), actionCollection());
    editCut->setToolTip(i18n("Cuts the currently selected entries from the phrase book and puts it to the clipboard"));
    editCut->setWhatsThis(i18n("Cuts the currently selected entries from the phrase book and puts it to the clipboard"));

    editCopy = KStandardAction::copy(this, SLOT(slotCopy()), actionCollection());
    editCopy->setToolTip(i18n("Copies the currently selected entry from the phrase book to the clipboard"));
    editCopy->setWhatsThis(i18n("Copies the currently selected entry from the phrase book to the clipboard"));

    editPaste = KStandardAction::paste(this, SLOT(slotPaste()), actionCollection());
    editPaste->setToolTip(i18n("Pastes the clipboard contents to current position"));
    editPaste->setWhatsThis(i18n("Pastes the clipboard contents at the current cursor position into the edit field."));

    editDelete = actionCollection()->addAction(QLatin1String("edit_delete"));
    editDelete->setIcon(KIcon(QLatin1String("edit-delete")));
    editDelete->setText(i18n("&Delete"));
    connect(editDelete, SIGNAL(triggered(bool)), this, SLOT(slotRemove()));
    editDelete->setToolTip(i18n("Deletes the selected entries from the phrase book"));
    editDelete->setWhatsThis(i18n("Deletes the selected entries from the phrase book"));

    // use the absolute path to your kmouthui.rc file for testing purpose in createGUI();
    createGUI(QLatin1String("phrasebookdialogui.rc"));
}

void PhraseBookDialog::initStandardPhraseBooks()
{
    StandardBookList bookPaths = PhraseBook::standardPhraseBooks();

    KActionMenu *parent = fileImportStandardBook;
    QStringList currentNamePath;
    currentNamePath << QLatin1String("x");
    QStack<KActionMenu *> stack;
    StandardBookList::iterator it;
    for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
        KUrl url;
        url.setPath((*it).filename);

        QString namePath = QLatin1String("x") + (*it).path;
        QStringList dirs = namePath.split(QLatin1Char('/'));

        QStringList::iterator it1 = currentNamePath.begin();
        QStringList::iterator it2 = dirs.begin();
        for (; (it1 != currentNamePath.end())
             && (it2 != dirs.end()) && (*it1 == *it2); ++it1, ++it2);

        for (; it1 != currentNamePath.end(); ++it1)
            parent = stack.pop();
        for (; it2 != dirs.end(); ++it2) {
            stack.push(parent);
            KActionMenu *newParent = actionCollection()->add<KActionMenu>(*it2);
            newParent->setText(*it2);
            parent->addAction(newParent);
            if (parent == fileImportStandardBook)
                toolbarImport->menu()->addAction(newParent);
            parent = newParent;
        }
        currentNamePath = dirs;

        KAction *book = new StandardPhraseBookInsertAction(
            url, (*it).name, this, SLOT(slotImportPhrasebook(KUrl)), actionCollection());
        parent->addAction(book);
        if (parent == fileImportStandardBook)
            toolbarImport->menu()->addAction(book);
    }
}

void PhraseBookDialog::connectEditor()
{
    connect(m_ui->lineEdit, SIGNAL(textChanged(QString)), SLOT(slotTextChanged(QString)));
    connect(m_ui->noKey, SIGNAL(clicked()), SLOT(slotNoKey()));
    connect(m_ui->customKey, SIGNAL(clicked()), SLOT(slotCustomKey()));
    connect(m_ui->keyButton, SIGNAL(keySequenceChanged(QKeySequence)),
            SLOT(slotKeySequenceChanged(QKeySequence)));
}

void PhraseBookDialog::disconnectEditor()
{
    disconnect(m_ui->lineEdit, SIGNAL(textChanged(QString)),
               this, SLOT(slotTextChanged(QString)));
    disconnect(m_ui->noKey, SIGNAL(clicked()),
               this, SLOT(slotNoKey()));
    disconnect(m_ui->customKey, SIGNAL(clicked()),
               this, SLOT(slotCustomKey()));
    disconnect(m_ui->keyButton, SIGNAL(keySequenceChanged(QKeySequence)),
               this, SLOT(slotKeySequenceChanged(QKeySequence)));
}

void PhraseBookDialog::selectionChanged()
{
    disconnectEditor();
    bool modified = phrasebookChanged;
    QModelIndex selected = m_ui->treeView->selectionModel()->currentIndex();
    QModelIndex shortcutIndex = selected.sibling(selected.row(), 1);
    QStandardItem *item = m_bookModel->itemFromIndex(selected);
    if (!m_ui->treeView->selectionModel()->hasSelection()) {
        m_ui->textLabel->setText(i18n("Text of the &phrase:"));
        m_ui->textLabel->setEnabled(false);
        m_ui->phrasebox->setEnabled(false);
        m_ui->lineEdit->setText(QLatin1String(""));
        m_ui->lineEdit->setEnabled(false);
        m_ui->shortcutLabel->setEnabled(false);
        m_ui->keyButton->clearKeySequence();
        m_ui->keyButton->setEnabled(false);
        m_ui->noKey->setChecked(false);
        m_ui->noKey->setEnabled(false);
        m_ui->customKey->setChecked(false);
        m_ui->customKey->setEnabled(false);
    } else if (!item->isDropEnabled()) {
        // It's a phrase, since drop isn't enabled
        m_ui->textLabel->setText(i18n("Text of the &phrase:"));
        m_ui->textLabel->setEnabled(true);
        m_ui->phrasebox->setEnabled(true);
        m_ui->lineEdit->setText(m_bookModel->data(selected, Qt::DisplayRole).toString());
        m_ui->lineEdit->setEnabled(true);
        m_ui->shortcutLabel->setEnabled(true);
        QString shortcut = m_bookModel->data(shortcutIndex, Qt::DisplayRole).toString();
        m_ui->keyButton->setKeySequence(QKeySequence(shortcut));
        if (shortcut.isEmpty() || shortcut.isNull()) {
            m_ui->noKey->setChecked(true);
        } else {
            m_ui->customKey->setChecked(true);
        }
        m_ui->keyButton->setEnabled(true);
        m_ui->noKey->setEnabled(true);
        m_ui->customKey->setEnabled(true);
    } else {
        // It's a book since drop is enabled
        m_ui->textLabel->setText(i18n("Name of the &phrase book:"));
        m_ui->textLabel->setEnabled(true);
        m_ui->phrasebox->setEnabled(true);
        m_ui->lineEdit->setText(m_bookModel->data(selected, Qt::DisplayRole).toString());
        m_ui->lineEdit->setEnabled(true);
        m_ui->shortcutLabel->setEnabled(false);
        m_ui->keyButton->clearKeySequence(); // Can't have a shortcut for a book
        m_ui->keyButton->setEnabled(false);
        m_ui->noKey->setChecked(false);
        m_ui->noKey->setEnabled(false);
        m_ui->customKey->setChecked(false);
        m_ui->customKey->setEnabled(false);
    }
    connectEditor();
    phrasebookChanged = modified;
}

bool PhraseBookDialog::queryClose()
{
    if (phrasebookChanged) {
        int answer = KMessageBox::questionYesNoCancel(this,
                     i18n("<qt>There are unsaved changes.<br />Do you want to apply the changes before closing the \"phrase book\" window or discard the changes?</qt>"),
                     i18n("Closing \"Phrase Book\" Window"),
                     KStandardGuiItem::apply(), KStandardGuiItem::discard(),
                     KStandardGuiItem::cancel(), QLatin1String("AutomaticSave"));
        if (answer == KMessageBox::Yes) {
            slotSave();
            return true;
        }
        return (answer == KMessageBox::No);
    }
    return true;
}

void PhraseBookDialog::slotTextChanged(const QString &s)
{
    QModelIndex selected = m_ui->treeView->selectionModel()->currentIndex();
    QModelIndex textIndex = selected.sibling(selected.row(), kTextColumn);
    if (textIndex.isValid()) {
        m_bookModel->setData(textIndex, s);
    }
}

void PhraseBookDialog::slotNoKey()
{
    m_ui->noKey->setChecked(true);
    m_ui->customKey->setChecked(false);  // This shouldn't be needed because of the groupbox... FIXME

    QModelIndex selected = m_ui->treeView->selectionModel()->currentIndex();
    QModelIndex shortcutIndex = selected.sibling(selected.row(), kShortcutColumn);
    if (shortcutIndex.isValid())
        m_bookModel->setData(shortcutIndex, QString());
    m_ui->keyButton->clearKeySequence();
}

void PhraseBookDialog::slotCustomKey()
{
    m_ui->keyButton->captureKeySequence();
}

void PhraseBookDialog::slotKeySequenceChanged(const QKeySequence& sequence)
{
    if (sequence.isEmpty()) {
        slotNoKey();
    } else
        setShortcut(sequence);
}

void PhraseBookDialog::setShortcut(const QKeySequence& sequence)
{
    // Check whether the shortcut is valid
//   const QList<QKeySequence> cutList = cut.toList();
//   for (int i = 0; i < cutList.count(); i++) {
//      const QKeySequence& seq = cutList[i];
//      //const KKey& key = seq.key(0);
//#ifdef __GNUC__
//#warning "kde 4 port it";
//#endif
//#if 0
//      if (key.modFlags() == 0 && key.sym() < 0x3000
//          && QChar(key.sym()).isLetterOrNumber())
//      {
//         QString s = i18n("In order to use the '%1' key as a shortcut, "
//                          "it must be combined with the "
//                          "Win, Alt, Ctrl, and/or Shift keys.", QChar(key.sym()));
//         KMessageBox::sorry( this, s, i18n("Invalid Shortcut Key") );
//         return;
//      }
//#endif
//   }
    QModelIndex selected = m_ui->treeView->selectionModel()->currentIndex();
    QModelIndex shortcutIndex = selected.sibling(selected.row(), kShortcutColumn);
    if (shortcutIndex.isValid())
        m_bookModel->setData(shortcutIndex, sequence.toString());
    //// If key isn't already in use,
    //   // Update display
    m_ui->noKey->setChecked(false);
    m_ui->customKey->setChecked(true);
    m_ui->keyButton->setKeySequence(sequence);
}

void PhraseBookDialog::contextMenuRequested(const QPoint &pos)
{
    QString name;
    if (m_ui->treeView->selectionModel()->hasSelection())
        name = QLatin1String("phrasebook_popup_sel");
    else
        name = QLatin1String("phrasebook_popup_nosel");

    QMenu *popup = (QMenu *)factory()->container(name, this);
    if (popup != 0) {
        popup->popup(m_ui->treeView->mapToGlobal(pos), 0);
    }
}

void PhraseBookDialog::slotRemove()
{
    if (m_ui->treeView->selectionModel()->hasSelection()) {
        QList<QModelIndex> selected = m_ui->treeView->selectionModel()->selectedRows();
        qSort(selected.begin(), selected.end());
        // Iterate over the rows backwards so we don't modify the .row of any indexes in selected.
        for (int i = selected.size() - 1; i >= 0; --i) {
            QModelIndex index = selected.at(i);
            m_bookModel->removeRows(index.row(), 1, index.parent());
        }
    }
}

void PhraseBookDialog::slotCut()
{
    slotCopy();
    slotRemove();
}

void PhraseBookDialog::slotCopy()
{
    QList<QModelIndex> selected = m_ui->treeView->selectionModel()->selectedRows();
    QString xml;
    foreach (const QModelIndex index, selected) {
        xml += serializeBook(index);
    }
    QMimeData *data = new QMimeData();
    data->setText(xml);
    QApplication::clipboard()->setMimeData(data);
}

void PhraseBookDialog::slotPaste()
{
    const QMimeData *data = QApplication::clipboard()->mimeData();
    QModelIndex index = m_ui->treeView->selectionModel()->currentIndex();
    QStandardItem *item = m_bookModel->itemFromIndex(index);
    while (item != NULL && !item->isDropEnabled())
        item = item->parent();
    QDomDocument document;
    document.setContent(data->text());
    QDomNode node = document.documentElement();
    deserializeBook(node, item);
}

QModelIndex PhraseBookDialog::getCurrentParent()
{
    QModelIndex currentIndex = m_ui->treeView->currentIndex();
    QStandardItem *item = m_bookModel->itemFromIndex(currentIndex);
    if (item != NULL && !item->isDropEnabled()) // If it's not a book
        currentIndex = currentIndex.parent();
    return currentIndex;
}

void PhraseBookDialog::focusNewItem(QModelIndex parent, QStandardItem *item)
{
    m_ui->treeView->expand(parent);
    QModelIndex newIndex = m_bookModel->indexFromItem(item);
    m_ui->treeView->setCurrentIndex(newIndex);

    m_ui->lineEdit->selectAll();
    m_ui->lineEdit->setFocus();
}

void PhraseBookDialog::slotAddPhrasebook()
{
    QModelIndex parentIndex = getCurrentParent();
    QStandardItem *parent = m_bookModel->itemFromIndex(parentIndex);
    QStandardItem *item = new QStandardItem(kPhraseBookIcon, i18n("(New Phrase Book)"));
    QStandardItem *shortcutItem = new QStandardItem();

    QList<QStandardItem*> items;
    items << item << shortcutItem;
    if (parent)
        parent->appendRow(items);
    else
        m_bookModel->appendRow(items);
    focusNewItem(parentIndex, item);
}

void PhraseBookDialog::slotAddPhrase()
{
    QModelIndex parentIndex = getCurrentParent();
    QStandardItem *parent = m_bookModel->itemFromIndex(parentIndex);
    QStandardItem *item = new QStandardItem(kPhraseIcon, i18n("(New Phrase)"));
    QStandardItem *shortcutItem = new QStandardItem();

    item->setDropEnabled(false);
    shortcutItem->setDropEnabled(false);

    QList<QStandardItem*> items;
    items << item << shortcutItem;
    if (parent)
        parent->appendRow(items);
    else
        m_bookModel->appendRow(items);
    focusNewItem(parentIndex, item);
}

void PhraseBookDialog::slotSave()
{
    QString standardBook = QStandardPaths::locate(QStandardPaths::DataLocation, QLatin1String("standard.phrasebook"));
    if (!standardBook.isNull() && !standardBook.isEmpty()) {
        QFile file(standardBook);
        file.open(QIODevice::WriteOnly);
        file.write(serializeBook(QModelIndex()).toUtf8());
        file.close();
        emit phrasebookConfirmed();
        phrasebookChanged = false;
        fileSave->setEnabled(false);
    }
}

void PhraseBookDialog::slotImportPhrasebook()
{
    KUrl url = KFileDialog::getOpenUrl(KUrl(),
                                       i18n("*.phrasebook|Phrase Books (*.phrasebook)\n*.txt|Plain Text Files (*.txt)\n*|All Files"), this, i18n("Import Phrasebook"));

    slotImportPhrasebook(url);
}

void PhraseBookDialog::slotImportPhrasebook(const KUrl &url)
{
    if (!url.isEmpty()) {
        QModelIndex parentIndex = getCurrentParent();
        QStandardItem *parentItem = m_bookModel->itemFromIndex(parentIndex);
        QFile file(url.toLocalFile());
        if (file.open(QIODevice::ReadOnly)) {
            QDomDocument document;
            document.setContent(&file);

            QDomNode node = document.documentElement();
            QStandardItem *item = 0;
            if (node.hasAttributes()) {
                // It has attributes, so add it directly
                item = deserializeBook(node, parentItem);
            } else {
                // It has no attributes, so add all it's children
                QDomNodeList nodes = node.childNodes();
                for (int i = 0; i < nodes.count(); ++i) {
                    QDomNode child = nodes.at(i);
                    item = deserializeBook(child, parentItem);
                }
            }
            focusNewItem(parentIndex, item);
        } else {
            KMessageBox::sorry(this, i18n("There was an error loading file\n%1", url.toLocalFile()));
        }
    }
}

void PhraseBookDialog::slotExportPhrasebook()
{
    // Get the current book or if a phrase is selected get it's parent book to export.
    QModelIndex parentIndex = getCurrentParent();
    QString content = kWholeBookXML.arg(serializeBook(parentIndex));

    KUrl url = KFileDialog::getSaveFileName(KUrl(), QLatin1String("*.phrasebook"), this, i18n("Export Phrase Book"));
    QFile file(url.toLocalFile());
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << content;
        file.close();
    } else {
        KMessageBox::sorry(this, i18n("There was an error saving file\n%1", url.url()));
    }
}

//void PhraseBookDialog::slotPrint()
//{
//   if (printer == 0) {
//     printer = new QPrinter();
//   }

//   QPrintDialog *printDialog = KdePrint::createPrintDialog(printer, this);

//   if (printDialog->exec()) {
//      PhraseBook book;
//      //treeView->fillBook (&book, treeView->hasSelectedItems());

//      book.print(printer);
//   }
//   delete printDialog;
//}

#include "phrasebookdialog.moc"
