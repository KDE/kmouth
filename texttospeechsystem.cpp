/***************************************************************************
                          texttospeechsystem.cpp  -  description
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

#include "texttospeechsystem.h"
#include <qregexp.h>
#include <qtextcodec.h>
#include <qptrlist.h>
#include <stdlib.h>
#include "speech.h"

TextToSpeechSystem::TextToSpeechSystem() {
   ttsCommand = "cat -";
   stdIn = true;
   codec = Speech::Local; // local encoding;
   buildCodecList();
}

TextToSpeechSystem::~TextToSpeechSystem() {
}

void TextToSpeechSystem::speak (QString text) {
   if (text.length() > 0)
      if (codec < Speech::UseCodec)
         (new Speech())->speak(ttsCommand, stdIn, text, codec, 0);
      else
         (new Speech())->speak(ttsCommand, stdIn, text, Speech::UseCodec,
                               codecList->at (codec - Speech::UseCodec));
}

void TextToSpeechSystem::readOptions (KConfig *config, const QString &langGroup) {
  config->setGroup(langGroup);
  ttsCommand = config->readEntry("Command", "cat -");
  stdIn = config->readBoolEntry("StdIn", true);

  QString codecString = config->readEntry("Codec", "Local");
  if (codecString == "Local")
     codec = Speech::Local;
  else if (codecString == "Latin1")
     codec = Speech::Latin1;
  else if (codecString == "Unicode")
     codec = Speech::Unicode;
  else {
     codec = Speech::Local;
     for (uint i = 0; i < codecList->count(); i++ )
        if (codecString == codecList->at(i)->name())
           codec = Speech::UseCodec + i;
  }
}

void TextToSpeechSystem::saveOptions (KConfig *config, const QString &langGroup) {
  config->setGroup(langGroup);
  config->writeEntry("Command", ttsCommand);
  config->writeEntry("StdIn", stdIn);
  if (codec == Speech::Local)
     config->writeEntry("Codec", "Local");
  else if (codec == Speech::Latin1)
     config->writeEntry("Codec", "Latin1");
  else if (codec == Speech::Unicode)
     config->writeEntry("Codec", "Unicode");
  else config->writeEntry("Codec",
         codecList->at (codec-Speech::UseCodec)->name());
  
}

void TextToSpeechSystem::buildCodecList () {
   codecList = new QPtrList<QTextCodec>;
   QTextCodec *codec;
   int i;
   for (i = 0; (codec = QTextCodec::codecForIndex(i)); i++)
      codecList->append (codec);
}

#include "texttospeechsystem.moc"

/*
 * $Log$
 * Revision 1.6  2002/12/04 16:22:02  gunnar
 * Include *.moc files
 *
 * Revision 1.5  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.4  2002/11/21 21:33:27  gunnar
 * Extended parameter dialog and added wizard for the first start
 *
 * Revision 1.3  2002/11/04 16:38:42  gunnar
 * Incorporated changes for version 0.5.1 into head branch
 *
 * Revision 1.2.2.1  2002/11/04 15:36:37  gunnar
 * combo box for character encoding added
 *
 * Revision 1.2  2002/10/02 14:55:33  gunnar
 * Fixed Speak-empty-phrase-crash bug and added some i18n() encodings
 *
 * Revision 1.1  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 */
