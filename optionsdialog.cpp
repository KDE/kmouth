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


#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qgrid.h>
#include <qtabwidget.h>
#include <qpixmap.h>
#include <qfile.h>

#include <kcombobox.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kcmodule.h>
#include <klibloader.h>
#include <kparts/componentfactory.h>

#include "optionsdialog.h"
#include "wordcompletion/wordcompletionwidget.h"

#include "texttospeechconfigurationwidget.h"
#include "speech.h"

PreferencesWidget::PreferencesWidget (QWidget *parent, const char *name)
   : PreferencesUI (parent, name)
{
   speakCombo->setCurrentItem (1);
   speak = false;

   closeCombo->setCurrentItem (2);
   save = 2;
}

PreferencesWidget::~PreferencesWidget() {
}

void PreferencesWidget::cancel() {
   if (speak)
      speakCombo->setCurrentItem (0);
   else
      speakCombo->setCurrentItem (1);
   closeCombo->setCurrentItem (save);
}

void PreferencesWidget::ok() {
   speak = speakCombo->currentItem () == 0;
   save  = closeCombo->currentItem ();
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
      speakCombo->setCurrentItem (0);
   else
      speakCombo->setCurrentItem (1);
   closeCombo->setCurrentItem (save);
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
   : KDialogBase(IconList, i18n("Configuration"), Ok|Apply|Cancel|Help, Ok,
                  parent, "configuration", false, true)
{
   setHelp ("config-dialog");

   QPixmap iconGeneral = KGlobal::iconLoader()->loadIcon("configure", KIcon::NoGroup, KIcon::SizeMedium);
   QGrid *pageGeneral = addGridPage (1, Qt::Horizontal, i18n("General Options"), QString::null, iconGeneral);
   
   tabCtl = new QTabWidget (pageGeneral, "general");

   behaviourWidget = new PreferencesWidget (tabCtl, "prefPage");
   behaviourWidget->layout()->setMargin(KDialog::marginHint());
   tabCtl->addTab (behaviourWidget, i18n("&Preferences"));
   
   commandWidget = new TextToSpeechConfigurationWidget (tabCtl, "ttsTab");
   commandWidget->layout()->setMargin(KDialog::marginHint());
   tabCtl->addTab (commandWidget, i18n("&Text-to-Speech"));
   
   QPixmap iconCompletion = KGlobal::iconLoader()->loadIcon("keyboard", KIcon::NoGroup, KIcon::SizeMedium);
   QGrid *pageCompletion = addGridPage (1, Qt::Horizontal, i18n("Word Completion"), QString::null, iconCompletion);
   completionWidget = new WordCompletionWidget(pageCompletion, "Word Completion widget");

   kttsd = loadKttsd();
   if (kttsd != 0) {
      QPixmap iconKttsd = KGlobal::iconLoader()->loadIcon("multimedia", KIcon::NoGroup, KIcon::SizeMedium);
      QGrid *pageKttsd = addGridPage (1, Qt::Horizontal, i18n("KTTSD Speech Service"),
                                      i18n("KDE Text-to-Speech Daemon Configuration"), iconKttsd);
      
      kttsd->reparent(pageKttsd,0,QPoint(0,0),true);
   }
}

OptionsDialog::~OptionsDialog() {
   unloadKttsd();
}

void OptionsDialog::slotCancel() {
   KDialogBase::slotCancel();
   commandWidget->cancel();
   behaviourWidget->cancel();
   completionWidget->load();
   if (kttsd != 0)
      kttsd->load ();
}

void OptionsDialog::slotOk() {
   KDialogBase::slotOk();
   commandWidget->ok();
   behaviourWidget->ok();
   completionWidget->save();
   emit configurationChanged();
   if (kttsd != 0)
      kttsd->save ();
   
}

void OptionsDialog::slotApply() {
   KDialogBase::slotApply();
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
            KCModule *module = KParts::ComponentFactory::createInstanceFromFactory<KCModule> (factory);
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
