/***************************************************************************
                          configwizard.cpp  -  description
                             -------------------
    begin                : Mit Nov 20 2002
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


#include "configwizard.h"
#include <qlayout.h>
#include <qlabel.h>
#include <klistview.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <ksconfig.h>
#include <kconfig.h>

#include "texttospeechconfigurationwidget.h"
#include "phrasebook/phrasebookdialog.h"
#include "wordcompletion/wordcompletion.h"
#include "wordcompletion/dictionarycreationwizard.h"

ConfigWizard::ConfigWizard (QWidget *parent, const char *name, KConfig *config)
             : KWizard(parent, name, true)
{
   setCaption (i18n("Initial Configuration - KMouth"));

   initCommandPage (config);
   initBookPage ();
   initCompletion (config);
}

ConfigWizard::~ConfigWizard() {
}

void ConfigWizard::initCommandPage(KConfig *config) {
   config->setGroup("TTS System");
   bool displayCommand = false;
   if (!config->hasKey("Command")) displayCommand = true;
   if (!config->hasKey("StdIn"))   displayCommand = true;
   if (!config->hasKey("Codec"))   displayCommand = true;

   if (displayCommand) {
      commandWidget = new TextToSpeechConfigurationWidget (this, "ttsPage");
      commandWidget->readOptions (config, "TTS System");
      addPage (commandWidget, i18n("Text-to-Speech Configuration"));
      setHelpEnabled (commandWidget, true);
      setFinishEnabled (commandWidget, true);
   }
   else
      commandWidget = 0;
}

void ConfigWizard::initBookPage() {
   QString standardBook = KApplication::kApplication()->dirs()->findResource("appdata", "standard.phrasebook");
   bool displayBook = (standardBook.isNull() || standardBook.isEmpty());

   if (displayBook) {
      bookWidget = new InitialPhraseBookWidget (this, "pbPage");
      addPage (bookWidget, i18n("Initial Phrase Book"));
      setHelpEnabled (bookWidget, true);
      setFinishEnabled (bookWidget, true);
      if (commandWidget != 0)
         setFinishEnabled (commandWidget, false);
   }
   else
      bookWidget = 0;
}

void ConfigWizard::initCompletion (KConfig *config) {
   if (!WordCompletion::isConfigured()) {
      QString dictionaryFile = KApplication::kApplication()->dirs()->findResource("appdata", "dictionary.txt");
      QFile file(dictionaryFile);
      if (file.exists()) {
         // If there is a word completion dictionary but no entry in the
         // configuration file, we need to add it there.
         config->setGroup("Dictionary 0");
         config->writeEntry ("Filename", "dictionary.txt");
         config->writeEntry ("Name",     "Default");
         config->writeEntry ("Language", QString::null);
         config->sync();
      }
   }
   if (!WordCompletion::isConfigured()) {
      completionWidget = new CompletionWizardWidget(this, "completionPage");
      addPage (completionWidget, i18n("Word Completion"));
      setHelpEnabled (completionWidget, true);
      setFinishEnabled (completionWidget, true);
   
      if (commandWidget != 0)
         setFinishEnabled (commandWidget, false);
      if (bookWidget != 0)
         setFinishEnabled (bookWidget, false);
   }
   else
      completionWidget = 0;
}

void ConfigWizard::saveConfig (KConfig *config) {
   if (commandWidget != 0) {
      commandWidget->ok();
      commandWidget->saveOptions (config, "TTS System");
   }

   if (bookWidget != 0)
      bookWidget->createBook();

   if (completionWidget != 0)
      completionWidget->ok (config);
}

bool ConfigWizard::requestConfiguration () {
   if (commandWidget != 0 || bookWidget != 0 || completionWidget != 0)
      return (exec() == QDialog::Accepted);
   else
      return false;
}

bool ConfigWizard::configurationNeeded () {
   return (commandWidget != 0 || bookWidget != 0 || completionWidget != 0);
}

void ConfigWizard::help () {
   KApplication::kApplication()->invokeHelp ("Wizard");
}

#include "configwizard.moc"
