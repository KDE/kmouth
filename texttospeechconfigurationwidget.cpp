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
#include <QTextCodec>
#include <q3ptrlist.h>
#include <QLayout>
#include <q3whatsthis.h>
#include <kcombobox.h>
#include <klocale.h>
#include <QLabel>
#include "speech.h"
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <kurlrequester.h>

TextToSpeechConfigurationWidget::TextToSpeechConfigurationWidget (QWidget *parent, const char *name)
   : QWidget (parent)
{
   setObjectName(name);
   setupUi(this);
   ttsSystem = new TextToSpeechSystem();

   buildCodecList();
}

TextToSpeechConfigurationWidget::~TextToSpeechConfigurationWidget() {
}

void TextToSpeechConfigurationWidget::buildCodecList () {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name() + ')';
   characterCodingBox->addItem (local, Speech::Local);
   characterCodingBox->addItem (i18n("Latin1"), Speech::Latin1);
   characterCodingBox->addItem (i18n("Unicode"), Speech::Unicode);
   for (int i = 0; i < ttsSystem->codecList->count(); i++ )
      characterCodingBox->addItem (ttsSystem->codecList->at(i)->name(), Speech::UseCodec + i);
}

void TextToSpeechConfigurationWidget::cancel() {
  urlReq->setUrl (ttsSystem->ttsCommand);
  stdInButton->setChecked (ttsSystem->stdIn);
  characterCodingBox->setCurrentIndex(ttsSystem->codec);
  useKttsd->setChecked (ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::ok() {
  ttsSystem->ttsCommand = urlReq->url().path();
  ttsSystem->stdIn = stdInButton->isChecked();
  ttsSystem->codec = characterCodingBox->currentIndex();
  ttsSystem->useKttsd = useKttsd->isChecked();
}

TextToSpeechSystem *TextToSpeechConfigurationWidget::getTTSSystem() const {
   return ttsSystem;
}

void TextToSpeechConfigurationWidget::readOptions (KConfig *config, const QString &langGroup) {
  ttsSystem->readOptions (config, langGroup);
  urlReq->setUrl (ttsSystem->ttsCommand);
  stdInButton->setChecked (ttsSystem->stdIn);
  characterCodingBox->setCurrentIndex(ttsSystem->codec);
  useKttsd->setChecked (ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::saveOptions (KConfig *config, const QString &langGroup) {
  ttsSystem->saveOptions (config, langGroup);
}

