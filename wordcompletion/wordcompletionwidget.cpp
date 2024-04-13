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

#include "wordcompletionwidget.h"

#include <QFileDialog>
#include <QStandardItemModel>
#include <QStandardPaths>
#include <QUrl>

#include <KConfigGroup>
#include <KIO/FileCopyJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSharedConfig>

#include "dictionarycreationwizard.h"
#include "wordcompletion.h"

WordCompletionWidget::WordCompletionWidget(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setupUi(this);
    setObjectName(QLatin1String(name));
    model = new QStandardItemModel(0, 2, this);
    dictionaryView->setModel(model);

    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();

    // Connect the signals from hte KCMKTTSDWidget to this class
    connect(addButton, &QAbstractButton::clicked, this, &WordCompletionWidget::addDictionary);
    connect(deleteButton, &QAbstractButton::clicked, this, &WordCompletionWidget::deleteDictionary);
    connect(moveUpButton, &QAbstractButton::clicked, this, &WordCompletionWidget::moveUp);
    connect(moveDownButton, &QAbstractButton::clicked, this, &WordCompletionWidget::moveDown);
    connect(exportButton, &QAbstractButton::clicked, this, &WordCompletionWidget::exportDictionary);

    connect(dictionaryView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &WordCompletionWidget::selectionChanged);
    connect(dictionaryName, &QLineEdit::textChanged, this, &WordCompletionWidget::nameChanged);
    connect(languageButton, &KLanguageButton::activated, this, &WordCompletionWidget::languageSelected);

    // Load the configuration from the file
    load();
}

/**
 * Destructor
 */
WordCompletionWidget::~WordCompletionWidget()
{
}

/***************************************************************************/

void WordCompletionWidget::load()
{
    model->clear();

    QStringList labels;
    labels << i18n("Dictionary") << i18n("Language");
    model->setHorizontalHeaderLabels(labels);

    // Set the group general for the configuration of kttsd itself (no plug ins)
    const QStringList groups = KSharedConfig::openConfig()->groupList();
    for (QStringList::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it)
        if ((*it).startsWith(QLatin1String("Dictionary "))) {
            KConfigGroup cg(KSharedConfig::openConfig(), *it);
            QString filename = cg.readEntry("Filename");
            QString languageTag = cg.readEntry("Language");
            QStandardItem *nameItem = new QStandardItem(cg.readEntry("Name"));
            nameItem->setData(filename);
            QStandardItem *languageItem = new QStandardItem(languageTag);
            QList<QStandardItem *> items;
            items.append(nameItem);
            items.append(languageItem);
            model->appendRow(items);
            if (!languageButton->contains(languageTag))
                languageButton->insertLanguage(languageTag, i18n("without name"));
        }

    // Clean up disc space
    for (QStringList::const_iterator it = newDictionaryFiles.constBegin(); it != newDictionaryFiles.constEnd(); ++it) {
        QString filename = QStandardPaths::locate(QStandardPaths::AppDataLocation, *it);
        if (!filename.isEmpty() && !filename.isNull())
            QFile::remove(filename);
    }
    newDictionaryFiles.clear();
}

void WordCompletionWidget::save()
{
    const QStringList groups = KSharedConfig::openConfig()->groupList();
    for (QStringList::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it)
        if ((*it).startsWith(QLatin1String("Dictionary ")))
            KSharedConfig::openConfig()->deleteGroup(*it);

    for (int row = 0; row < model->rowCount(); ++row) {
        const QStandardItem *nameItem = model->item(row, 0);
        const QStandardItem *languageItem = model->item(row, 1);
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Dictionary %1").arg(row));
        cg.writeEntry("Filename", nameItem->data().toString());
        cg.writeEntry("Name", nameItem->text());
        cg.writeEntry("Language", languageItem->text());
    }
    KSharedConfig::openConfig()->sync();

    // Clean up disc space
    for (QStringList::const_iterator it = removedDictionaryFiles.constBegin(); it != removedDictionaryFiles.constEnd(); ++it) {
        QString filename = QStandardPaths::locate(QStandardPaths::AppDataLocation, *it);
        if (!filename.isEmpty() && !filename.isNull())
            QFile::remove(filename);
    }
    removedDictionaryFiles.clear();
}

/***************************************************************************/

void WordCompletionWidget::addDictionary()
{
    QStringList dictionaryNames;
    QStringList dictionaryFiles;
    QStringList dictionaryLanguages;
    for (int row = 0; row < model->rowCount(); ++row) {
        const QStandardItem *nameItem = model->item(row, 0);
        const QStandardItem *languageItem = model->item(row, 1);
        dictionaryNames += nameItem->text();
        dictionaryFiles += nameItem->data().toString();
        dictionaryLanguages += languageItem->text();
    }
    DictionaryCreationWizard *wizard = new DictionaryCreationWizard(this, dictionaryNames, dictionaryFiles, dictionaryLanguages);
    if (wizard->exec() == QDialog::Accepted) {
        QString filename = wizard->createDictionary();
        newDictionaryFiles += filename;
        QString languageTag = wizard->language();
        if (!languageButton->contains(languageTag)) {
            languageButton->insertLanguage(languageTag, i18n("without name"));
        }
        QStandardItem *nameItem = new QStandardItem(wizard->name());
        nameItem->setData(filename);
        QStandardItem *languageItem = new QStandardItem(languageTag);
        QList<QStandardItem *> items;
        items.append(nameItem);
        items.append(languageItem);
        model->appendRow(items);
    }
    delete wizard;
}

void WordCompletionWidget::deleteDictionary()
{
    int row = dictionaryView->currentIndex().row();
    const QStandardItem *nameItem = model->item(row, 0);

    if (nameItem != nullptr) {
        removedDictionaryFiles += nameItem->data().toString();
        qDeleteAll(model->takeRow(row));
    }
}

void WordCompletionWidget::moveUp()
{
    int row = dictionaryView->currentIndex().row();
    if (row > 0) {
        QList<QStandardItem *> items = model->takeRow(row);
        model->insertRow(row - 1, items);
        dictionaryView->setCurrentIndex(model->index(row - 1, 0));
    }
}

void WordCompletionWidget::moveDown()
{
    int row = dictionaryView->currentIndex().row();
    if (row < model->rowCount() - 1) {
        QList<QStandardItem *> items = model->takeRow(row);
        model->insertRow(row + 1, items);
        dictionaryView->setCurrentIndex(model->index(row + 1, 0));
    }
}

void WordCompletionWidget::exportDictionary()
{
    const QStandardItem *nameItem = model->item(dictionaryView->currentIndex().row(), 0);

    if (nameItem != nullptr) {
        QUrl url = QFileDialog::getSaveFileUrl(this, i18n("Export Dictionary"));
        if (url.isEmpty() || !url.isValid())
            return;

        if (url.isLocalFile() && QFile::exists(url.toLocalFile())) {
            if (KMessageBox::warningContinueCancel(nullptr,
                                                   QStringLiteral("<qt>%1</qt>")
                                                       .arg(i18n("The file %1 already exists. "
                                                                 "Do you want to overwrite it?",
                                                                 url.url())),
                                                   i18n("File Exists"),
                                                   KGuiItem(i18n("&Overwrite")))
                == KMessageBox::Cancel) {
                return;
            }
        }
        QUrl src;
        src.setPath(QStandardPaths::locate(QStandardPaths::AppDataLocation, nameItem->data().toString()));
        KIO::FileCopyJob *job = KIO::file_copy(src, url);
        job->exec();
    }
}

void WordCompletionWidget::selectionChanged()
{
    QModelIndex current = dictionaryView->currentIndex();
    deleteButton->setEnabled(current.isValid());
    exportButton->setEnabled(current.isValid());
    selectedDictionaryDetails->setEnabled(current.isValid());
    moveUpButton->setEnabled(current.isValid() && current.row() > 0);
    moveDownButton->setEnabled(current.isValid() && current.row() < model->rowCount() - 1);

    if (current.isValid()) {
        const QStandardItem *nameItem = model->item(current.row(), 0);
        const QStandardItem *languageItem = model->item(current.row(), 1);

        dictionaryName->setText(nameItem->text());
        languageButton->setCurrentItem(languageItem->text());
    } else {
        dictionaryName->clear();
    }
}

void WordCompletionWidget::nameChanged(const QString &text)
{
    const QStandardItem *nameItem = model->item(dictionaryView->currentIndex().row(), 0);

    if (nameItem != nullptr) {
        QString old = nameItem->text();

        if (old != text) {
            QStandardItem *newItem = new QStandardItem(text);
            newItem->setData(nameItem->data());
            model->setItem(dictionaryView->currentIndex().row(), 0, newItem);
            Q_EMIT changed(true);
        }
    }
}

void WordCompletionWidget::languageSelected()
{
    const QStandardItem *languageItem = model->item(dictionaryView->currentIndex().row(), 1);

    if (languageItem != nullptr) {
        QString old = languageItem->text();
        QString text = languageButton->current();

        if (old != text) {
            QStandardItem *newItem = new QStandardItem(text);
            model->setItem(dictionaryView->currentIndex().row(), 1, newItem);
            Q_EMIT changed(true);
        }
    }
}

#include "moc_wordcompletionwidget.cpp"
