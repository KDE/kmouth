/***************************************************************************
                          wordcompletionwidget.cpp  -  description
                             -------------------
    begin                : Tue Apr 29 2003
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

#include "wordcompletionwidget.h"
#include "wordcompletion.h"
#include "dictionarycreationwizard.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QStandardItemModel>

#include <QDebug>

#include <klineedit.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <klanguagebutton.h>

WordCompletionWidget::WordCompletionWidget(QWidget *parent, const char *name)
: QWidget(parent)
{
    setupUi(this);
    setObjectName( QLatin1String( name ) );
    model = new QStandardItemModel(0, 2, this);
    dictionaryView->setModel(model);

    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();

    // Connect the signals from hte KCMKTTSDWidget to this class
    connect (addButton, SIGNAL (clicked()), this, SLOT(addDictionary()) );
    connect (deleteButton, SIGNAL (clicked()), this, SLOT (deleteDictionary()) );
    connect (moveUpButton, SIGNAL (clicked()), this, SLOT (moveUp()) );
    connect (moveDownButton, SIGNAL (clicked()), this, SLOT (moveDown()) );
    connect (exportButton, SIGNAL (clicked()), this, SLOT (exportDictionary()) );

    connect (dictionaryView->selectionModel(), SIGNAL (selectionChanged(QItemSelection, QItemSelection)),
             this, SLOT (selectionChanged()) );
    connect (dictionaryName, SIGNAL (textChanged(QString)), this, SLOT (nameChanged(QString)) );
    connect (languageButton, SIGNAL (activated(QString)), this, SLOT (languageSelected()) );

    // Object for the KCMKTTSD configuration
    config = new KConfig(QLatin1String( "kmouthrc" ));

    // Load the configuration from the file
    load();
    qDebug() << "horizontal header data is " << model->headerData(0, Qt::Horizontal) << model->headerData(1, Qt::Horizontal);
}

/**
 * Destructor
 */
WordCompletionWidget::~WordCompletionWidget() {
    delete config;
}

/***************************************************************************/

void WordCompletionWidget::load() {
   model->clear();

   QStringList labels;
   labels << i18n("Dictionary") << i18n("Language");
   model->setHorizontalHeaderLabels(labels);

   // Set the group general for the configuration of kttsd itself (no plug ins)
   const QStringList groups = config->groupList();
   for (QStringList::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it)
      if ((*it).startsWith (QLatin1String("Dictionary "))) {
         KConfigGroup cg (config, *it);
         QString filename = cg.readEntry("Filename");
         QString languageTag = cg.readEntry("Language");
         QStandardItem *nameItem = new QStandardItem(cg.readEntry("Name"));
         nameItem->setData(filename);
         QStandardItem *languageItem = new QStandardItem(languageTag);
         QList<QStandardItem*> items;
         items.append(nameItem);
         items.append(languageItem);
         model->appendRow(items);
         if (!languageButton->contains(languageTag))
            languageButton->insertLanguage(languageTag, i18n("without name"));
      }

   // Clean up disc space
   for (QStringList::const_iterator it = newDictionaryFiles.constBegin(); it != newDictionaryFiles.constEnd(); ++it) {
      QString filename = KGlobal::dirs()->findResource ("appdata", *it);
      if (!filename.isEmpty() && !filename.isNull())
         QFile::remove (filename);
   }
   newDictionaryFiles.clear();
}

void WordCompletionWidget::save() {
   const QStringList groups = config->groupList();
   for (QStringList::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it)
      if ((*it).startsWith (QLatin1String("Dictionary ")))
         config->deleteGroup (*it);

   int row = 0;
   for (int row = 0; row < model->rowCount(); ++row) {
      const QStandardItem *nameItem = model->item(row, 0);
      const QStandardItem *languageItem = model->item(row, 1);
      KConfigGroup cg (config, QString(QLatin1String( "Dictionary %1" )).arg(row));
      cg.writeEntry ("Filename", nameItem->data().toString());
      cg.writeEntry ("Name",     nameItem->text ());
      cg.writeEntry ("Language", languageItem->text ());
   }
   config->sync();

   // Clean up disc space
   for (QStringList::const_iterator it = removedDictionaryFiles.constBegin(); it != removedDictionaryFiles.constEnd(); ++it) {
      QString filename = KGlobal::dirs()->findResource ("appdata", *it);
      if (!filename.isEmpty() && !filename.isNull())
         QFile::remove (filename);
   }
   removedDictionaryFiles.clear();
}

/***************************************************************************/

void WordCompletionWidget::addDictionary() {
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
   DictionaryCreationWizard *wizard = new DictionaryCreationWizard (this, "Dictionary creation wizard", dictionaryNames, dictionaryFiles, dictionaryLanguages);
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
      QList<QStandardItem*> items;
      items.append(nameItem);
      items.append(languageItem);
      model->appendRow(items);
   }
   delete wizard;
}

void WordCompletionWidget::deleteDictionary() {
   int row = dictionaryView->currentIndex().row();
   const QStandardItem *nameItem = model->item(row, 0);

   if (nameItem != 0) {
      removedDictionaryFiles += nameItem->data().toString();
      qDeleteAll(model->takeRow(row));
   }
}

void WordCompletionWidget::moveUp() {
   int row = dictionaryView->currentIndex().row();
   if (row > 0) {
      QList<QStandardItem*> items = model->takeRow(row);
      model->insertRow(row - 1, items);
      dictionaryView->setCurrentIndex(model->index(row - 1, 0));
   }
}

void WordCompletionWidget::moveDown() {
   int row = dictionaryView->currentIndex().row();
   if (row < model->rowCount() - 1) {
      QList<QStandardItem*> items = model->takeRow(row);
      model->insertRow(row + 1, items);
      dictionaryView->setCurrentIndex(model->index(row + 1, 0));
   }
}

void WordCompletionWidget::exportDictionary() {
   const QStandardItem *nameItem = model->item(dictionaryView->currentIndex().row(), 0);

   if (nameItem != 0) {
      KUrl url = KFileDialog::getSaveUrl(QString(), QString(), this, i18n("Export Dictionary"));
      if (url.isEmpty() || !url.isValid())
         return;

      if (KIO::NetAccess::exists(url, KIO::NetAccess::DestinationSide, this)) {
         if (KMessageBox::warningContinueCancel(0,QString(QLatin1String( "<qt>%1</qt>" )).arg(i18n("The file %1 already exists. "
                                                          "Do you want to overwrite it?", url.url())),i18n("File Exists"),KGuiItem(i18n("&Overwrite")))==KMessageBox::Cancel) {
            return;
         }
      }
      KUrl src;
      src.setPath( KGlobal::dirs()->findResource ("appdata", nameItem->data().toString()) );
      KIO::NetAccess::file_copy (src, url, this);
   }
}

void WordCompletionWidget::selectionChanged() {
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

void WordCompletionWidget::nameChanged (const QString &text) {
   const QStandardItem *nameItem = model->item(dictionaryView->currentIndex().row(), 0);

   if (nameItem != 0) {
      QString old = nameItem->text();

      if (old != text) {
         QStandardItem *newItem = new QStandardItem(text);
         newItem->setData(nameItem->data());
         model->setItem(dictionaryView->currentIndex().row(), 0, newItem);
         emit changed(true);
      }
   }
}

void WordCompletionWidget::languageSelected () {
   const QStandardItem *languageItem = model->item(dictionaryView->currentIndex().row(), 1);

   if (languageItem != 0) {
      QString old = languageItem->text();
      QString text = languageButton->current();

      if (old != text) {
         QStandardItem *newItem = new QStandardItem(text);
         model->setItem(dictionaryView->currentIndex().row(), 1, newItem);
         emit changed(true);
      }
   }
}

#include "wordcompletionwidget.moc"
