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


#include "texttospeechsystem.h"
#include <QTextCodec>
#include <stdlib.h>
#include <QtDBus>
#include <kapplication.h>
#include <kconfig.h>

#include "speech.h"

TextToSpeechSystem::TextToSpeechSystem() {
   ttsCommand = "";
   stdIn = true;
   useKttsd = true;
   codec = Speech::Local; // local encoding;
   buildCodecList();
}

TextToSpeechSystem::~TextToSpeechSystem() {
   delete codecList;
}

bool kttsdSay (const QString &text, const QString &language) {
   QDBusInterface kdesktop("org.kde.kttsd", "/org/kde/KSpeech", "org.kde.KSpeech");
   QDBusReply<bool> reply = kdesktop.call("sayWarning", text, language);
	return reply;
}

void TextToSpeechSystem::speak (const QString &text, const QString &language) {
   if (text.length() > 0) {
      if (useKttsd) {
         if (kttsdSay(text, language))
            return;
      }

      if (codec < Speech::UseCodec)
         (new Speech())->speak(ttsCommand, stdIn, text, language, codec, 0);
      else
         (new Speech())->speak(ttsCommand, stdIn, text, language, Speech::UseCodec,
                               codecList->at (codec - Speech::UseCodec));
   }
}

void TextToSpeechSystem::readOptions (KConfig *config, const QString &langGroup) {
  config->setGroup(langGroup);
  ttsCommand = config->readPathEntry("Command");
  stdIn = config->readEntry("StdIn", QVariant(true)).toBool();
  useKttsd = config->readEntry("useKttsd", QVariant(true)).toBool();

  QString codecString = config->readEntry("Codec", "Local");
  if (codecString == "Local")
     codec = Speech::Local;
  else if (codecString == "Latin1")
     codec = Speech::Latin1;
  else if (codecString == "Unicode")
     codec = Speech::Unicode;
  else {
     codec = Speech::Local;
     for (int i = 0; i < codecList->count(); i++ )
        if (codecString == codecList->at(i)->name())
           codec = Speech::UseCodec + i;
  }
}

void TextToSpeechSystem::saveOptions (KConfig *config, const QString &langGroup) {
  config->setGroup(langGroup);
  config->writePathEntry("Command", ttsCommand);
  config->writeEntry("StdIn", stdIn);
  config->writeEntry("useKttsd", useKttsd);
  if (codec == Speech::Local)
     config->writeEntry("Codec", "Local");
  else if (codec == Speech::Latin1)
     config->writeEntry("Codec", "Latin1");
  else if (codec == Speech::Unicode)
     config->writeEntry("Codec", "Unicode");
  else {
     QString codeName = codecList->at (codec-Speech::UseCodec)->name();
     config->writeEntry("Codec", codeName);
  }
}

void TextToSpeechSystem::buildCodecList () {
   codecList = new QList<QTextCodec*>;
   QList<QByteArray> availableCodecs = QTextCodec::availableCodecs();
   for (int i = 0; i < availableCodecs.count(); ++i) {
       QTextCodec *codec = QTextCodec::codecForName(availableCodecs[i]);
       codecList->append (codec);
   }
}

#include "texttospeechsystem.moc"
