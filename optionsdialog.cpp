/***************************************************************************
                          optionsdialog.cpp  -  description
                             -------------------
    begin                : Don Nov 21 2002
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


#include <QLayout>
#include <QLabel>
#include <q3whatsthis.h>
#include <q3grid.h>
#include <QTabWidget>
#include <QPixmap>
#include <QFile>

#include <kcombobox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kcmodule.h>
#include <klibloader.h>
#include <kicon.h>
#include <kparts/componentfactory.h>

#include "optionsdialog.h"
#include "wordcompletion/wordcompletionwidget.h"

#include "texttospeechconfigurationwidget.h"
#include "speech.h"

PreferencesWidget::PreferencesWidget (QWidget *parent, const char *name)
   : QWidget (parent)
{
   setObjectName(name);
   setupUi(this);
   speakCombo->setCurrentIndex (1);
   speak = false;

   closeCombo->setCurrentIndex (2);
   save = 2;
}

PreferencesWidget::~PreferencesWidget() {
}

void PreferencesWidget::cancel() {
   if (speak)
      speakCombo->setCurrentIndex (0);
   else
      speakCombo->setCurrentIndex (1);
   closeCombo->setCurrentIndex (save);
}

void PreferencesWidget::ok() {
   speak = speakCombo->currentIndex () == 0;
   save  = closeCombo->currentIndex ();
}

void PreferencesWidget::readOptions (KConfig *config) {
   config->setGroup("Preferences");
   if (config->hasKey("AutomaticSpeak"))
      if (config->readEntry ("AutomaticSpeak") == "Yes")
         speak = true;
      else
         speak = false;
   else
      speak = false;

   config->setGroup("Notification Messages");
   if (config->hasKey("AutomaticSave"))
      if (config->readEntry ("AutomaticSave") == "Yes")
         save = 0;
      else
         save = 1;
   else
      save = 2;

   if (speak)
      speakCombo->setCurrentIndex (0);
   else
      speakCombo->setCurrentIndex (1);
   closeCombo->setCurrentIndex (save);
}

void PreferencesWidget::saveOptions (KConfig *config) {
   config->setGroup("Preferences");
   if (speak)
      config->writeEntry ("AutomaticSpeak", "Yes");
   else
      config->writeEntry ("AutomaticSpeak", "No");

   config->setGroup("Notification Messages");
   if (save == 0)
      config->writeEntry ("AutomaticSave", "Yes");
   else if (save == 1)
      config->writeEntry ("AutomaticSave", "No");
   else
      config->deleteEntry ("AutomaticSave");
}

bool PreferencesWidget::isSpeakImmediately () {
   return speak;
}

/***************************************************************************/

OptionsDialog::OptionsDialog (QWidget *parent)
   : KPageDialog(parent)
{
   setCaption(i18n("Configuration"));
   setButtons(KDialog::Ok|KDialog::Apply|KDialog::Cancel|KDialog::Help);
   setFaceType(KPageDialog::List);
   setHelp ("config-dialog");

   
	   //addGridPage (1, Qt::Horizontal, i18n("General Options"), QString(), iconGeneral);
   
   tabCtl = new QTabWidget();
   tabCtl->setObjectName("general");

   behaviourWidget = new PreferencesWidget (tabCtl, "prefPage");
   behaviourWidget->layout()->setMargin(KDialog::marginHint());
   tabCtl->addTab (behaviourWidget, i18n("&Preferences"));
   
   commandWidget = new TextToSpeechConfigurationWidget (tabCtl, "ttsTab");
   commandWidget->layout()->setMargin(KDialog::marginHint());
   tabCtl->addTab (commandWidget, i18n("&Text-to-Speech"));
   
   KPageWidgetItem *pageGeneral = new KPageWidgetItem(tabCtl, i18n("General Options"));
   pageGeneral->setHeader(i18n("General Options"));
   pageGeneral->setIcon(KIcon("configure"));
   addPage(pageGeneral);
   
   completionWidget = new WordCompletionWidget(0, "Word Completion widget");
   KPageWidgetItem *pageCompletion = new KPageWidgetItem(completionWidget, i18n("Word Completion"));
   pageCompletion->setHeader(i18n("Word Completion"));
   pageCompletion->setIcon(KIcon("keyboard"));
   addPage(pageCompletion);

   kttsd = loadKttsd();
   if (kttsd != 0) {
      KPageWidgetItem *pageKttsd = new KPageWidgetItem(kttsd, i18n("KTTSD Speech Service"));
      pageKttsd->setIcon(KIcon("multimedia"));
      pageKttsd->setHeader(i18n("KDE Text-to-Speech Daemon Configuration"));
      addPage(pageKttsd);
   }
   
   setDefaultButton(KDialog::Cancel);
   
   connect(this, SIGNAL(okClicked()), this, SLOT(slotOk()));
   connect(this, SIGNAL(cancelClicked()), this, SLOT(slotCancel()));
   connect(this, SIGNAL(applyClicked()), this, SLOT(slotApply()));
}

OptionsDialog::~OptionsDialog() {
   unloadKttsd();
}

void OptionsDialog::slotCancel() {
//   KDialog::slotCancel();
   commandWidget->cancel();
   behaviourWidget->cancel();
   completionWidget->load();
   if (kttsd != 0)
      kttsd->load ();
}

void OptionsDialog::slotOk() {
//   KDialog::slotOk();
   commandWidget->ok();
   behaviourWidget->ok();
   completionWidget->save();
   emit configurationChanged();
   if (kttsd != 0)
      kttsd->save ();
   
}

void OptionsDialog::slotApply() {
//   KDialog::slotApply();
   commandWidget->ok();
   behaviourWidget->ok();
   completionWidget->save();
   emit configurationChanged();
   if (kttsd != 0)
      kttsd->save ();
}

TextToSpeechSystem *OptionsDialog::getTTSSystem() const {
   return commandWidget->getTTSSystem();
}

void OptionsDialog::readOptions (KConfig *config) {
   commandWidget->readOptions (config, "TTS System");
   behaviourWidget->readOptions (config);
}

void OptionsDialog::saveOptions (KConfig *config) {
   commandWidget->saveOptions (config, "TTS System");
   behaviourWidget->saveOptions (config);
   config->sync();
}

bool OptionsDialog::isSpeakImmediately () {
   return behaviourWidget->isSpeakImmediately ();
}

KCModule *OptionsDialog::loadKttsd () {
   KLibLoader *loader = KLibLoader::self();

   QString libname = "kcm_kttsd";
   KLibrary *lib = loader->library(QFile::encodeName(libname));

   if (lib == 0) {
      libname = "libkcm_kttsd";
      lib = loader->library(QFile::encodeName("libkcm_kttsd"));
   }

   if (lib != 0) {
      QString initSym("init_");
      initSym += libname;

      if (lib->hasSymbol(QFile::encodeName(initSym))) {
         // Reuse "lib" instead of letting createInstanceFromLibrary recreate it
         KLibFactory *factory = lib->factory();
         if (factory != 0) {
            KCModule *module = factory->create<KCModule> (factory);
            if (module)
                return module;
         }
      }

      lib->unload();
   }
   return 0;
}

void OptionsDialog::unloadKttsd () {
  KLibLoader *loader = KLibLoader::self();
  loader->unloadLibrary(QFile::encodeName("libkcm_kttsd"));
  loader->unloadLibrary(QFile::encodeName("kcm_kttsd"));
}

#include "optionsdialog.moc"
