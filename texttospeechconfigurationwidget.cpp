/***************************************************************************
                          texttospeechconfigurationdialog.cpp  -  description
                             -------------------
    begin                : Son Sep 8 2002
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


#include "texttospeechconfigurationwidget.h"
#include <kconfig.h>
#include <tqtextcodec.h>
#include <tqptrlist.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <kcombobox.h>
#include <klocale.h>
#include <tqlabel.h>
#include "speech.h"
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqcheckbox.h>
#include <kurlrequester.h>

TextToSpeechConfigurationWidget::TextToSpeechConfigurationWidget (TQWidget *tqparent, const char *name)
   : texttospeechconfigurationui (tqparent, name)
{
   ttsSystem = new TextToSpeechSystem();

   urlReq->setShowLocalProtocol (false);
   buildCodecList();
}

TextToSpeechConfigurationWidget::~TextToSpeechConfigurationWidget() {
}

void TextToSpeechConfigurationWidget::buildCodecList () {
   TQString local = i18n("Local")+" (";
   local += TQTextCodec::codecForLocale()->name();
   local += ")";
   characterCodingBox->insertItem (local, Speech::Local);
   characterCodingBox->insertItem (i18n("Latin1"), Speech::Latin1);
   characterCodingBox->insertItem (i18n("Unicode"), Speech::Unicode);
   for (uint i = 0; i < ttsSystem->codecList->count(); i++ )
      characterCodingBox->insertItem (ttsSystem->codecList->at(i)->name(), Speech::UseCodec + i);
}

void TextToSpeechConfigurationWidget::cancel() {
  urlReq->setURL (ttsSystem->ttsCommand);
  stdInButton->setChecked (ttsSystem->stdIn);
  characterCodingBox->setCurrentItem(ttsSystem->codec);
  useKttsd->setChecked (ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::ok() {
  ttsSystem->ttsCommand = urlReq->url();
  ttsSystem->stdIn = stdInButton->isChecked();
  ttsSystem->codec = characterCodingBox->currentItem();
  ttsSystem->useKttsd = useKttsd->isChecked();
}

TextToSpeechSystem *TextToSpeechConfigurationWidget::getTTSSystem() const {
   return ttsSystem;
}

void TextToSpeechConfigurationWidget::readOptions (KConfig *config, const TQString &langGroup) {
  ttsSystem->readOptions (config, langGroup);
  urlReq->setURL (ttsSystem->ttsCommand);
  stdInButton->setChecked (ttsSystem->stdIn);
  characterCodingBox->setCurrentItem(ttsSystem->codec);
  useKttsd->setChecked (ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::saveOptions (KConfig *config, const TQString &langGroup) {
  ttsSystem->saveOptions (config, langGroup);
}

