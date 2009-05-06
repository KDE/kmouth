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

#include <k3listview.h>
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

class DictionaryListItem : public K3ListViewItem {
public:
   DictionaryListItem (Q3ListView *parent, QString filename, QString name, QString language, QString languageCode)
   : K3ListViewItem (parent, name) {
      setFilename (filename);
      setLanguage (language, languageCode);
   }
   DictionaryListItem (Q3ListView *parent, QString filename, QString name, QString languageCode)
   : K3ListViewItem (parent, name) {
      setFilename (filename);
      setLanguage (languageCode);
   }
   DictionaryListItem (Q3ListView *parent, Q3ListViewItem *after, QString filename, QString name, QString languageCode)
   : K3ListViewItem (parent, after, name) {
      setFilename (filename);
      setLanguage (languageCode);
   }
   ~DictionaryListItem () {
   }

   QString filename() {
      return myFilename;
   }

   QString languageCode() {
      return myLanguageCode;
   }

   void setFilename(QString filename) {
      myFilename = filename;
   }

   void setLanguage (QString languageCode) {
      QString name = KGlobal::locale()->languageCodeToName(languageCode);
      if (name.isEmpty())
      {
          name = i18n("without name");
      }
      setLanguage (name + " (" + languageCode + ')', languageCode);
   }

   void setLanguage (QString name, QString languageCode) {
      myLanguageCode = languageCode;
      setText (1, name);
   }

private:
   QString myFilename;
   QString myLanguageCode;
};

/***************************************************************************/

WordCompletionWidget::WordCompletionWidget(QWidget *parent, const char *name)
: QWidget(parent)
{
    setupUi(this);
    setObjectName(name);
    dictionaryList->setSorting (-1); // no sorted list
    
    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();
    
    // Connect the signals from hte KCMKTTSDWidget to this class
    connect (addButton, SIGNAL (clicked()), this, SLOT(addDictionary()) );
    connect (deleteButton, SIGNAL (clicked()), this, SLOT (deleteDictionary()) );
    connect (moveUpButton, SIGNAL (clicked()), this, SLOT (moveUp()) );
    connect (moveDownButton, SIGNAL (clicked()), this, SLOT (moveDown()) );
    connect (exportButton, SIGNAL (clicked()), this, SLOT (exportDictionary()) );

    connect (dictionaryList, SIGNAL (selectionChanged()), this, SLOT (selectionChanged()) );
    connect (dictionaryName, SIGNAL (textChanged (const QString &)), this, SLOT (nameChanged (const QString &)) );
    connect (languageButton, SIGNAL (activated (const QString &)), this, SLOT (languageSelected()) );

    // Object for the KCMKTTSD configuration
    config = new KConfig("kmouthrc");

    // Load the configuration from the file
    load();
}

/**
 * Destructor
 */
WordCompletionWidget::~WordCompletionWidget() {
    delete config;
}

/***************************************************************************/

void WordCompletionWidget::load() {
   dictionaryList->clear();

   // Set the group general for the configuration of kttsd itself (no plug ins)
   const QStringList groups = config->groupList();
   DictionaryListItem *last = 0;
   for (QStringList::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it)
      if ((*it).startsWith (QString("Dictionary "))) {
		 KConfigGroup cg (config, *it);
         QString languageTag = cg.readEntry("Language");
         last = new DictionaryListItem (dictionaryList, last,
                                        cg.readEntry("Filename"),
                                        cg.readEntry("Name"),
                                        languageTag);
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
      if ((*it).startsWith (QString("Dictionary ")))
         config->deleteGroup (*it);

   int number = 0;
   Q3ListViewItemIterator it(dictionaryList);
   while (it.current()) {
      DictionaryListItem *item = dynamic_cast<DictionaryListItem*>(it.current());
      if (item != 0) {
		 KConfigGroup cg (config, QString("Dictionary %1").arg(number));
         cg.writeEntry ("Filename", item->filename());
         cg.writeEntry ("Name",     item->text (0));
         cg.writeEntry ("Language", item->languageCode());
         number++;
      }
      ++it;
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
   Q3ListViewItemIterator it(dictionaryList);
   while (it.current()) {
      DictionaryListItem *item = dynamic_cast<DictionaryListItem*>(it.current());
      if (item != 0) {
         dictionaryNames += item->text (0);
         dictionaryFiles += item->filename();
         dictionaryLanguages += item->languageCode();
      }
      ++it;
   }
   DictionaryCreationWizard *wizard = new DictionaryCreationWizard (this, "Dictionary creation wizard", dictionaryNames, dictionaryFiles, dictionaryLanguages);
   if (wizard->exec() == QDialog::Accepted) {
      QString filename = wizard->createDictionary();
      newDictionaryFiles += filename;
      QString languageTag = wizard->language();
      if (!languageButton->contains(languageTag)) {
         languageButton->insertLanguage(languageTag, i18n("without name"));
      }
      K3ListViewItem *item = new DictionaryListItem (dictionaryList,
                      filename, wizard->name(), languageTag);
      dictionaryList->setSelected(item, true);
   }
   delete wizard;
}

void WordCompletionWidget::deleteDictionary() {
   DictionaryListItem *item = dynamic_cast<DictionaryListItem*>(dictionaryList->selectedItem ());

   if (item != 0) {
      removedDictionaryFiles += item->filename();
      delete item;
   }
}

void WordCompletionWidget::moveUp() {
   Q3ListViewItem *item = dictionaryList->selectedItem ();

   if (item != 0) {
      Q3ListViewItem *above = item->itemAbove();

      if (above != 0) {
         above->moveItem (item);
      }
   }
}

void WordCompletionWidget::moveDown() {
   Q3ListViewItem *item = dictionaryList->selectedItem ();

   if (item != 0) {
      Q3ListViewItem *next = item->itemBelow();

      if (next != 0) {
         item->moveItem (next);
      }
   }
}

void WordCompletionWidget::exportDictionary() {
   DictionaryListItem *item = dynamic_cast<DictionaryListItem*>(dictionaryList->selectedItem ());

   if (item != 0) {
      KUrl url = KFileDialog::getSaveUrl(QString(), QString(), this, i18n("Export Dictionary"));
      if (url.isEmpty() || !url.isValid())
         return;

      if (KIO::NetAccess::exists(url, KIO::NetAccess::DestinationSide, this)) {
         if (KMessageBox::warningContinueCancel(0,QString("<qt>%1</qt>").arg(i18n("The file %1 already exists. "
                                                          "Do you want to overwrite it?", url.url())),i18n("File Exists"),KGuiItem(i18n("&Overwrite")))==KMessageBox::Cancel) {
            return;
         }
      }
      KUrl src;
      src.setPath( KGlobal::dirs()->findResource ("appdata", item->filename()) );
      KIO::NetAccess::copy (src, url, this);
   }
}

void WordCompletionWidget::selectionChanged() {
   DictionaryListItem *item = dynamic_cast<DictionaryListItem*>(dictionaryList->selectedItem ());

   if (item != 0) {
      deleteButton->setEnabled(true);
      moveUpButton->setEnabled(true);
      moveDownButton->setEnabled(true);
      exportButton->setEnabled(true);
      selectedDictionaryDetails->setEnabled(true);
      languageLabel->setEnabled(true);
      dictionaryNameLabel->setEnabled(true);
      dictionaryName->setEnabled(true);
      languageButton->setEnabled(true);

      dictionaryName->setText(item->text(0));
      languageButton->setCurrentItem(item->languageCode());
   }
   else {
      deleteButton->setEnabled(false);
      moveUpButton->setEnabled(false);
      moveDownButton->setEnabled(false);
      exportButton->setEnabled(false);
      selectedDictionaryDetails->setEnabled(false);
      languageLabel->setEnabled(false);
      dictionaryNameLabel->setEnabled(false);
      dictionaryName->setEnabled(false);
      languageButton->setEnabled(false);

      dictionaryName->setText("");
   }
}

void WordCompletionWidget::nameChanged (const QString &text) {
   Q3ListViewItem *item = dictionaryList->selectedItem ();

   if (item != 0) {
      QString old = item->text(0);

      if (old != text) {
         item->setText(0, text);
         emit changed(true);
      }
   }
}

void WordCompletionWidget::languageSelected () {
   DictionaryListItem *item = dynamic_cast<DictionaryListItem*>(dictionaryList->selectedItem ());

   if (item != 0) {
      QString old = item->text(1);
      QString text = languageButton->current();

      if (old != text) {
         item->setLanguage(languageButton->current(), text);
         emit changed(true);
      }
   }
}

#include "wordcompletionwidget.moc"
