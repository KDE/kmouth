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

// $Id$

#include <qlayout.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <ktabctl.h>
#include <kcombobox.h>
#include <klocale.h>

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

/*
 * $Log$
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.5  2003/01/17 16:03:00  gunnar
 * Help buutons added and small bug when aborting the wizard fixed
 *
 * Revision 1.4  2002/12/04 16:22:02  gunnar
 * Include *.moc files
 *
 * Revision 1.3  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.2  2002/11/22 08:48:34  gunnar
 * Implemented functionality that belongs to the new options in the options dialog
 *
 * Revision 1.1  2002/11/21 21:33:26  gunnar
 * Extended parameter dialog and added wizard for the first start
 *
 * Revision 1.6  2002/11/20 10:55:44  gunnar
 * Improved the keyboard accessibility
 *
 * Revision 1.5  2002/11/04 16:38:42  gunnar
 * Incorporated changes for version 0.5.1 into head branch
 *
 * Revision 1.4.2.1  2002/11/04 15:36:37  gunnar
 * combo box for character encoding added
 *
 * Revision 1.4  2002/10/07 17:09:33  gunnar
 * What's this? texts added
 *
 * Revision 1.3  2002/10/02 14:55:33  gunnar
 * Fixed Speak-empty-phrase-crash bug and added some i18n() encodings
 *
 * Revision 1.2  2002/09/08 19:29:42  gunnar
 * Configuration dialog improved
 *
 * Revision 1.1  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 */
