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

// $Id$

#include "texttospeechconfigurationwidget.h"
#include <kconfig.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <qlayout.h>
#include <qwhatsthis.h>
#include <kcombobox.h>
#include <klocale.h>
#include <qlabel.h>
#include "speech.h"

TextToSpeechConfigurationWidget::TextToSpeechConfigurationWidget (QWidget *parent, const char *name)
   : texttospeechconfigurationui (parent, name)
{
   ttsSystem = new TextToSpeechSystem();

   urlReq->setShowLocalProtocol (false);
   buildCodecList();
}

TextToSpeechConfigurationWidget::~TextToSpeechConfigurationWidget() {
}

void TextToSpeechConfigurationWidget::buildCodecList () {
   QString local = i18n("Local")+" (";
   local += QTextCodec::codecForLocale()->name();
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

void TextToSpeechConfigurationWidget::readOptions (KConfig *config, const QString &langGroup) {
  ttsSystem->readOptions (config, langGroup);
  urlReq->setURL (ttsSystem->ttsCommand);
  stdInButton->setChecked (ttsSystem->stdIn);
  characterCodingBox->setCurrentItem(ttsSystem->codec);
  useKttsd->setChecked (ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::saveOptions (KConfig *config, const QString &langGroup) {
  ttsSystem->saveOptions (config, langGroup);
}

/*
 * $Log$
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.2  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.1  2002/11/21 21:33:27  gunnar
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
