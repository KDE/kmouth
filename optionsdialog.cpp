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
#include <ktabctl.h>
#include <kcombobox.h>
#include <klocale.h>
#include <kconfig.h>

#include "optionsdialog.h"

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
   : KDialogBase(parent, "configuration", false,
                 i18n("Configuration"), Ok|Apply|Cancel|Help, Ok, true)
{
   tabCtl = new KTabCtl (this, "tabs");
   setMainWidget(tabCtl);
   setHelp ("config-dialog");

   commandWidget = new TextToSpeechConfigurationWidget (tabCtl, "ttsTab");
   commandWidget->layout()->setMargin(KDialog::marginHint());
   tabCtl->addTab (commandWidget, i18n("Text-to-Speech"));

   behaviourWidget = new PreferencesWidget (tabCtl, "prefPage");
   behaviourWidget->layout()->setMargin(KDialog::marginHint());
   tabCtl->addTab (behaviourWidget, i18n("Preferences"));
}

OptionsDialog::~OptionsDialog() {
}


void OptionsDialog::slotCancel() {
   KDialogBase::slotCancel();
   commandWidget->cancel();
   behaviourWidget->cancel();
}

void OptionsDialog::slotOk() {
   KDialogBase::slotOk();
   commandWidget->ok();
   behaviourWidget->ok();
   emit configurationChanged();
}

void OptionsDialog::slotApply() {
   KDialogBase::slotApply();
   commandWidget->ok();
   behaviourWidget->ok();
   emit configurationChanged();
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

#include "optionsdialog.moc"
