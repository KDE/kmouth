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

#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqradiobutton.h>
#include <tqlineedit.h>
#include <tqcombobox.h>
#include <tqtextcodec.h>
#include <tqwhatsthis.h>

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

DictionaryCreationWizard::DictionaryCreationWizard (TQWidget *parent, const char *name,
               TQStringList dictionaryNames, TQStringList dictionaryFiles,
               TQStringList dictionaryLanguages)
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
    TQWhatsThis::add (dirWidget->urlLabel, i18n("With this input field you specify which directory you want to load for creating the new dictionary."));
   dirWidget->url->setMode(KFile::Directory);
    TQWhatsThis::add (dirWidget->url, i18n("With this input field you specify which directory you want to load for creating the new dictionary."));
   buildCodecCombo (dirWidget->encodingCombo);

   kdeDocWidget= new KDEDocSourceUI (this, "KDE documentation source page");
   addPage (kdeDocWidget, i18n("Source of New Dictionary (2)"));

   mergeWidget = new MergeWidget (this, "merge source page", dictionaryNames, dictionaryFiles, dictionaryLanguages);
   addPage (mergeWidget, i18n("Source of New Dictionary (2)"));
   
   connect (creationSource->fileButton,    TQT_SIGNAL (toggled(bool)), this, TQT_SLOT(calculateAppropriate(bool)) );
   connect (creationSource->directoryButton,TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(calculateAppropriate(bool)) );
   connect (creationSource->kdeDocButton,  TQT_SIGNAL (toggled(bool)), this, TQT_SLOT(calculateAppropriate(bool)) );
   connect (creationSource->mergeButton,   TQT_SIGNAL (toggled(bool)), this, TQT_SLOT(calculateAppropriate(bool)) );
   connect (creationSource->emptyButton,   TQT_SIGNAL (toggled(bool)), this, TQT_SLOT(calculateAppropriate(bool)) );

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
   codecList = new TQPtrList<TQTextCodec>;
   TQTextCodec *codec;
   int i;
   for (i = 0; (codec = TQTextCodec::codecForIndex(i)); i++)
      codecList->append (codec);
}

void DictionaryCreationWizard::buildCodecCombo (TQComboBox *combo) {
   TQString local = i18n("Local")+" (";
   local += TQTextCodec::codecForLocale()->name();
   local += ")";
   combo->insertItem (local, 0);
   combo->insertItem (i18n("Latin1"), 1);
   combo->insertItem (i18n("Unicode"), 2);
   for (uint i = 0; i < codecList->count(); i++ )
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

TQString DictionaryCreationWizard::createDictionary() {
   WordList::WordMap map;
   TQString dicFile = "";
   KProgressDialog *pdlg = WordList::progressDialog();

   if (creationSource->mergeButton->isChecked()) {
      map = WordList::mergeFiles (mergeWidget->mergeParameters(), pdlg);
      dicFile = TQString::null;
   }
   else if (creationSource->emptyButton->isChecked()) {
      dicFile = TQString::null;
   }
   else if (creationSource->fileButton->isChecked()) {
      TQString filename = fileWidget->url->url();
      int encoding = fileWidget->encodingCombo->currentItem();
      if (fileWidget->spellCheckBox->isChecked())
         dicFile = fileWidget->ooDictURL->url();
      switch (encoding) {
      case 0:
         map = WordList::parseFile (filename, TQTextStream::Locale, 0, pdlg);
         break;
      case 1:
         map = WordList::parseFile (filename, TQTextStream::Latin1, 0, pdlg);
         break;
      case 2:
         map = WordList::parseFile (filename, TQTextStream::Unicode, 0, pdlg);
         break;
      default:
         map = WordList::parseFile (filename, (TQTextStream::Encoding)0, codecList->at(encoding-3), pdlg);
      }
   }
   else if (creationSource->directoryButton->isChecked()) {
      TQString directory = dirWidget->url->url();
      int encoding = dirWidget->encodingCombo->currentItem();
      if (dirWidget->spellCheckBox->isChecked())
         dicFile = dirWidget->ooDictURL->url();
      switch (encoding) {
      case 0:
         map = WordList::parseDir (directory, TQTextStream::Locale, 0, pdlg);
         break;
      case 1:
         map = WordList::parseDir (directory, TQTextStream::Latin1, 0, pdlg);
         break;
      case 2:
         map = WordList::parseDir (directory, TQTextStream::Unicode, 0, pdlg);
         break;
      default:
         map = WordList::parseDir (directory, (TQTextStream::Encoding)0, codecList->at(encoding-3), pdlg);
      }
   }
   else { // creationSource->kdeDocButton must be checked
      TQString language = kdeDocWidget->languageButton->currentTag();
      if (kdeDocWidget->spellCheckBox->isChecked())
         dicFile = kdeDocWidget->ooDictURL->url();
      map = WordList::parseKDEDoc (language, pdlg);
   }
   
   if (!dicFile.isEmpty() && !dicFile.isNull())
      map = WordList::spellCheck (map, dicFile, pdlg);
   pdlg->close();
   delete pdlg;
   
   int dictnumber = 0;
   TQString filename;
   TQString dictionaryFile;
   do {
      dictnumber++;
      filename = TQString("wordcompletion%1.dict").arg(dictnumber);
      dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", filename);
   }
   while (KStandardDirs::exists(dictionaryFile));
   
   dictionaryFile = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/") + filename;
   if (WordList::saveWordList (map, dictionaryFile))
      return filename;
   else
      return "";
}

TQString DictionaryCreationWizard::name() {
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

TQString DictionaryCreationWizard::language() {
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
      return fileWidget->languageButton->currentTag();
   }
   else if (creationSource->directoryButton->isChecked()) {
      return dirWidget->languageButton->currentTag();
   }
   else { // creationSource->kdeDocButton must be checked
      return kdeDocWidget->languageButton->currentTag();
   }
}

/***************************************************************************/

MergeWidget::MergeWidget(KWizard *parent, const char *name,
               TQStringList dictionaryNames, TQStringList dictionaryFiles,
               TQStringList dictionaryLanguages)
: TQScrollView (parent, name) {
   dictionaries.setAutoDelete (false);
   weights.setAutoDelete (false);

   TQWidget *contents = new TQWidget(viewport());
   addChild(contents);
   TQGridLayout *layout = new TQGridLayout (contents);
   setResizePolicy (TQScrollView::AutoOneFit);
   layout->setColStretch (0, 0);
   layout->setColStretch (1, 1);

   int row = 0;
   TQStringList::Iterator nIt = dictionaryNames.begin();
   TQStringList::Iterator fIt = dictionaryFiles.begin();
   TQStringList::Iterator lIt = dictionaryLanguages.begin();
   for (; nIt != dictionaryNames.end(); ++nIt, ++fIt, ++lIt) {
      TQCheckBox *checkbox = new TQCheckBox(*nIt, contents);
      KIntNumInput *numInput = new KIntNumInput(contents);
      layout->addWidget (checkbox, row, 0);
      layout->addWidget (numInput, row, 1);
      
      checkbox->setChecked (true);
      numInput->setRange (1, 100, 10, true);
      numInput->setValue (100);
      connect (checkbox, TQT_SIGNAL (toggled(bool)), numInput, TQT_SLOT(setEnabled(bool)));
      
      dictionaries.insert(*fIt, checkbox);
      weights.insert(*fIt, numInput);
      languages [*fIt] = *lIt;
      row++;
   }
}

MergeWidget::~MergeWidget() {
}

TQMap <TQString, int> MergeWidget::mergeParameters () {
   TQMap <TQString, int> files;
   TQDictIterator<TQCheckBox> it(dictionaries);
   for (; it.current(); ++it) {
      if (it.current()->isChecked()) {
         TQString name = it.currentKey();
         TQString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", name);
         files[dictionaryFile] = weights[name]->value();
      }
   }

   return files;
}

TQString MergeWidget::language () {
   TQDictIterator<TQCheckBox> it(dictionaries);
   for (; it.current(); ++it) {
      if (it.current()->isChecked()) {
         return languages [it.currentKey()];
      }
   }

   return TQString::null;
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

   TQString language = languageButton->currentTag();
   map = WordList::parseKDEDoc (language, pdlg);

   if (spellCheckBox->isChecked())
      map = WordList::spellCheck (map, ooDictURL->url(), pdlg);

   pdlg->close();
   delete pdlg;
   
   TQString filename;
   TQString dictionaryFile;
   
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
