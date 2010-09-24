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
#include <stdlib.h>

#include <QtCore/QTextCodec>
#include <QtDBus/QtDBus>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kspeech.h>
#include <kdebug.h>
#include <kspeech_interface.h>

#include "speech.h"

TextToSpeechSystem::TextToSpeechSystem() {
   stdIn = true;
   useKttsd = true;
   codec = Speech::Local; // local encoding;
   buildCodecList();
}

TextToSpeechSystem::~TextToSpeechSystem() {
   delete codecList;
}

bool kttsdSay (const QString &text, const QString &language) {
   // TODO: Would be better to save off this QDBusInterface pointer and
   // set defaults only once.
   org::kde::KSpeech kspeech(QLatin1String( "org.kde.KSpeech" ), QLatin1String( "/KSpeech" ), QDBusConnection::sessionBus());
   kspeech.setApplicationName(QLatin1String( "KMouth" ));
   kspeech.setDefaultTalker(language);

   // FIXME: language is incorrect.
   kDebug() << "kttsdSay: language = " << language;
   kspeech.setDefaultPriority(KSpeech::jpWarning);
   QDBusReply<int> val = kspeech.say(text, 0);

   return (val>0);
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
  KConfigGroup cg(config, langGroup);
  ttsCommand = cg.readPathEntry("Command", QString());
  stdIn = cg.readEntry("StdIn", true);
  useKttsd = cg.readEntry("useKttsd", true);

  QString codecString = cg.readEntry("Codec", "Local");
  if (codecString == QLatin1String( "Local" ))
     codec = Speech::Local;
  else if (codecString == QLatin1String( "Latin1" ))
     codec = Speech::Latin1;
  else if (codecString == QLatin1String( "Unicode" ))
     codec = Speech::Unicode;
  else {
     codec = Speech::Local;
     for (int i = 0; i < codecList->count(); i++ )
        if (codecString == QLatin1String( codecList->at(i)->name() ))
           codec = Speech::UseCodec + i;
  }
}

void TextToSpeechSystem::saveOptions (KConfig *config, const QString &langGroup) {
  KConfigGroup cg(config, langGroup);
  cg.writePathEntry("Command", ttsCommand);
  cg.writeEntry("StdIn", stdIn);
  cg.writeEntry("useKttsd", useKttsd);
  if (codec == Speech::Local)
     cg.writeEntry("Codec", "Local");
  else if (codec == Speech::Latin1)
     cg.writeEntry("Codec", "Latin1");
  else if (codec == Speech::Unicode)
     cg.writeEntry("Codec", "Unicode");
  else {
     QString codeName = QLatin1String( codecList->at (codec-Speech::UseCodec)->name() );
     cg.writeEntry("Codec", codeName);
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
