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

#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qlineedit.h>
#include <qcombobox.h>
#include <qtextcodec.h>

//Added by qt3to4:
#include <QTextStream>
#include <QGridLayout>

#include <klistview.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kcombobox.h>
#include <ksconfig.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kprogress.h>
#include <kconfig.h>

#include "dictionarycreationwizard.h"
#include "klanguagebutton.h"
#include "creationsourceui.h"
#include "creationsourcedetailsui.h"
#include "kdedocsourceui.h"
#include "wordlist.h"

DictionaryCreationWizard::DictionaryCreationWizard (QWidget *parent, const char *name,
               QStringList dictionaryNames, QStringList dictionaryFiles,
               QStringList dictionaryLanguages)
   : KWizard (parent, name)
{
   buildCodecList ();
   
   creationSource = new CreationSourceUI (this, "source page");
   addPage (creationSource, i18n("Source of New Dictionary (1)"));
   setHelpEnabled (creationSource, false);
   setFinishEnabled (creationSource, false);

   fileWidget= new CreationSourceDetailsUI (this, "file source page");
   addPage (fileWidget, i18n("Source of New Dictionary (2)"));
   buildCodecCombo (fileWidget->encodingCombo);

   dirWidget= new CreationSourceDetailsUI (this, "directory source page");
   addPage (dirWidget, i18n("Source of New Dictionary (2)"));
   dirWidget->urlLabel->setText (i18n("&Directory:"));
    dirWidget->urlLabel->setWhatsThis( i18n("With this input field you specify which directory you want to load for creating the new dictionary."));
   dirWidget->url->setMode(KFile::Directory);
    dirWidget->url->setWhatsThis( i18n("With this input field you specify which directory you want to load for creating the new dictionary."));
   buildCodecCombo (dirWidget->encodingCombo);

   kdeDocWidget= new KDEDocSourceUI (this, "KDE documentation source page");
   addPage (kdeDocWidget, i18n("Source of New Dictionary (2)"));

   mergeWidget = new MergeWidget (this, "merge source page", dictionaryNames, dictionaryFiles, dictionaryLanguages);
   addPage (mergeWidget, i18n("Source of New Dictionary (2)"));
   
   connect (creationSource->fileButton,    SIGNAL (toggled(bool)), this, SLOT(calculateAppropriate(bool)) );
   connect (creationSource->directoryButton,SIGNAL(toggled(bool)), this, SLOT(calculateAppropriate(bool)) );
   connect (creationSource->kdeDocButton,  SIGNAL (toggled(bool)), this, SLOT(calculateAppropriate(bool)) );
   connect (creationSource->mergeButton,   SIGNAL (toggled(bool)), this, SLOT(calculateAppropriate(bool)) );
   connect (creationSource->emptyButton,   SIGNAL (toggled(bool)), this, SLOT(calculateAppropriate(bool)) );

   calculateAppropriate (true);
}

DictionaryCreationWizard::~DictionaryCreationWizard () {
   delete codecList;
   removePage (fileWidget);   delete fileWidget;
   removePage (dirWidget);    delete dirWidget;
   removePage (kdeDocWidget); delete kdeDocWidget;
   removePage (mergeWidget);  delete mergeWidget;
}

void DictionaryCreationWizard::buildCodecList () {
   codecList = new QList<QTextCodec*>;
   QList<QByteArray> availableCodecs = QTextCodec::availableCodecs();
   for (int i = 0; i < availableCodecs.count(); ++i) {
       QTextCodec *codec = QTextCodec::codecForName(availableCodecs[i]);
       codecList->append (codec);
   }
}

void DictionaryCreationWizard::buildCodecCombo (QComboBox *combo) {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name();
   local += ")";
   combo->insertItem (local, 0);
   combo->insertItem (i18n("Latin1"), 1);
   combo->insertItem (i18n("Unicode"), 2);
   for (int i = 0; i < codecList->count(); i++ )
      combo->insertItem (codecList->at(i)->name(), i+3);
}

void DictionaryCreationWizard::calculateAppropriate (bool) {
   if (creationSource->mergeButton->isChecked()) {
      setFinishEnabled (creationSource, false);
      removePage (fileWidget);
      removePage (dirWidget);
      removePage (kdeDocWidget);
      addPage (mergeWidget, i18n("Source of New Dictionary (2)"));
      setHelpEnabled (mergeWidget, false);
      setFinishEnabled (mergeWidget, true);
   }
   else if (creationSource->emptyButton->isChecked()) {
      removePage (fileWidget);
      removePage (dirWidget);
      removePage (kdeDocWidget);
      removePage (mergeWidget);
      setFinishEnabled (creationSource, true);
   }
   else if (creationSource->fileButton->isChecked()) {
      setFinishEnabled (creationSource, false);
      removePage (dirWidget);
      removePage (kdeDocWidget);
      removePage (mergeWidget);
      addPage (fileWidget, i18n("Source of New Dictionary (2)"));
      setHelpEnabled (fileWidget, false);
      setFinishEnabled (fileWidget, true);
   }
   else if (creationSource->directoryButton->isChecked()) {
      setFinishEnabled (creationSource, false);
      removePage (fileWidget);
      removePage (kdeDocWidget);
      removePage (mergeWidget);
      addPage (dirWidget, i18n("Source of New Dictionary (2)"));
      setHelpEnabled (dirWidget, false);
      setFinishEnabled (dirWidget, true);
   }
   else { // creationSource->kdeDocButton must be checked
      setFinishEnabled (creationSource, false);
      removePage (fileWidget);
      removePage (dirWidget);
      removePage (mergeWidget);
      addPage (kdeDocWidget, i18n("Source of New Dictionary (2)"));
      setHelpEnabled (kdeDocWidget, false);
      setFinishEnabled (kdeDocWidget, true);
   }
}

QString DictionaryCreationWizard::createDictionary() {
   WordList::WordMap map;
   QString dicFile = "";
   KProgressDialog *pdlg = WordList::progressDialog();

   if (creationSource->mergeButton->isChecked()) {
      map = WordList::mergeFiles (mergeWidget->mergeParameters(), pdlg);
      dicFile.clear();
   }
   else if (creationSource->emptyButton->isChecked()) {
      dicFile.clear();
   }
   else if (creationSource->fileButton->isChecked()) {
      QString filename = fileWidget->url->url();
      int encoding = fileWidget->encodingCombo->currentItem();
      if (fileWidget->spellCheckBox->isChecked())
         dicFile = fileWidget->ooDictURL->url();
      switch (encoding) {
      case 0:
         map = WordList::parseFile (filename, QTextStream::Locale, 0, pdlg);
         break;
      case 1:
         map = WordList::parseFile (filename, QTextStream::Latin1, 0, pdlg);
         break;
      case 2:
         map = WordList::parseFile (filename, QTextStream::Unicode, 0, pdlg);
         break;
      default:
         map = WordList::parseFile (filename, (QTextStream::Encoding)0, codecList->at(encoding-3), pdlg);
      }
   }
   else if (creationSource->directoryButton->isChecked()) {
      QString directory = dirWidget->url->url();
      int encoding = dirWidget->encodingCombo->currentItem();
      if (dirWidget->spellCheckBox->isChecked())
         dicFile = dirWidget->ooDictURL->url();
      switch (encoding) {
      case 0:
         map = WordList::parseDir (directory, QTextStream::Locale, 0, pdlg);
         break;
      case 1:
         map = WordList::parseDir (directory, QTextStream::Latin1, 0, pdlg);
         break;
      case 2:
         map = WordList::parseDir (directory, QTextStream::Unicode, 0, pdlg);
         break;
      default:
         map = WordList::parseDir (directory, (QTextStream::Encoding)0, codecList->at(encoding-3), pdlg);
      }
   }
   else { // creationSource->kdeDocButton must be checked
      QString language = kdeDocWidget->languageButton->current();
      if (kdeDocWidget->spellCheckBox->isChecked())
         dicFile = kdeDocWidget->ooDictURL->url();
      map = WordList::parseKDEDoc (language, pdlg);
   }
   
   if (!dicFile.isEmpty() && !dicFile.isNull())
      map = WordList::spellCheck (map, dicFile, pdlg);
   pdlg->close();
   delete pdlg;
   
   int dictnumber = 0;
   QString filename;
   QString dictionaryFile;
   do {
      dictnumber++;
      filename = QString("wordcompletion%1.dict").arg(dictnumber);
      dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", filename);
   }
   while (KStandardDirs::exists(dictionaryFile));
   
   dictionaryFile = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/") + filename;
   if (WordList::saveWordList (map, dictionaryFile))
      return filename;
   else
      return "";
}

QString DictionaryCreationWizard::name() {
   if (creationSource->mergeButton->isChecked()) {
      return i18n("Merge result");
   }
   else if (creationSource->emptyButton->isChecked()) {
      return i18n("In the sense of a blank word list", "Empty list");
   }
   else if (creationSource->fileButton->isChecked()) {
      return fileWidget->url->url();
   }
   else if (creationSource->directoryButton->isChecked()) {
      return dirWidget->url->url();
   }
   else { // creationSource->kdeDocButton must be checked
      return i18n("KDE Documentation");
   }
}

QString DictionaryCreationWizard::language() {
   if (creationSource->mergeButton->isChecked()) {
      return mergeWidget->language();
   }
   else if (creationSource->emptyButton->isChecked()) {
      if (KGlobal::locale())
         return KGlobal::locale()->language();
      else
         return KLocale::defaultLanguage();
   }
   else if (creationSource->fileButton->isChecked()) {
      return fileWidget->languageButton->current();
   }
   else if (creationSource->directoryButton->isChecked()) {
      return dirWidget->languageButton->current();
   }
   else { // creationSource->kdeDocButton must be checked
      return kdeDocWidget->languageButton->current();
   }
}

/***************************************************************************/

MergeWidget::MergeWidget(KWizard *parent, const char *name,
               QStringList dictionaryNames, QStringList dictionaryFiles,
               QStringList dictionaryLanguages)
: Q3ScrollView (parent, name) {
   dictionaries.setAutoDelete (false);
   weights.setAutoDelete (false);

   QWidget *contents = new QWidget(viewport());
   addChild(contents);
   QGridLayout *layout = new QGridLayout (contents);
   setResizePolicy (Q3ScrollView::AutoOneFit);
   layout->setColStretch (0, 0);
   layout->setColStretch (1, 1);

   int row = 0;
   QStringList::Iterator nIt = dictionaryNames.begin();
   QStringList::Iterator fIt = dictionaryFiles.begin();
   QStringList::Iterator lIt = dictionaryLanguages.begin();
   for (; nIt != dictionaryNames.end(); ++nIt, ++fIt, ++lIt) {
      QCheckBox *checkbox = new QCheckBox(*nIt, contents);
      KIntNumInput *numInput = new KIntNumInput(contents);
      layout->addWidget (checkbox, row, 0);
      layout->addWidget (numInput, row, 1);
      
      checkbox->setChecked (true);
      numInput->setRange (1, 100, 10, true);
      numInput->setValue (100);
      connect (checkbox, SIGNAL (toggled(bool)), numInput, SLOT(setEnabled(bool)));
      
      dictionaries.insert(*fIt, checkbox);
      weights.insert(*fIt, numInput);
      languages [*fIt] = *lIt;
      row++;
   }
}

MergeWidget::~MergeWidget() {
}

QMap <QString, int> MergeWidget::mergeParameters () {
   QMap <QString, int> files;
   Q3DictIterator<QCheckBox> it(dictionaries);
   for (; it.current(); ++it) {
      if (it.current()->isChecked()) {
         QString name = it.currentKey();
         QString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", name);
         files[dictionaryFile] = weights[name]->value();
      }
   }

   return files;
}

QString MergeWidget::language () {
   Q3DictIterator<QCheckBox> it(dictionaries);
   for (; it.current(); ++it) {
      if (it.current()->isChecked()) {
         return languages [it.currentKey()];
      }
   }

   return QString();
}

/***************************************************************************/

CompletionWizardWidget::CompletionWizardWidget (KWizard *parent, const char *name)
   : KDEDocSourceUI (parent, name) {
}

CompletionWizardWidget::~CompletionWizardWidget() {
}

void CompletionWizardWidget::ok (KConfig *config) {
   WordList::WordMap map;
   KProgressDialog *pdlg = WordList::progressDialog();

   QString language = languageButton->current();
   map = WordList::parseKDEDoc (language, pdlg);

   if (spellCheckBox->isChecked())
      map = WordList::spellCheck (map, ooDictURL->url(), pdlg);

   pdlg->close();
   delete pdlg;
   
   QString filename;
   QString dictionaryFile;
   
   dictionaryFile = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/") + "wordcompletion1.dict";
   if (WordList::saveWordList (map, dictionaryFile)) {
      config->setGroup("Dictionary 0");
      config->writeEntry ("Filename", "wordcompletion1.dict");
      config->writeEntry ("Name",     i18n("Default"));
      config->writeEntry ("Language", language);
      config->sync();
   }
}

#include "dictionarycreationwizard.moc"
