/***************************************************************************
                          speech.h  -  description
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

#ifndef SPEECH_H
#define SPEECH_H

#include <tqobject.h>
#include <tqstring.h>
#include <kprocess.h>
#include <ktempfile.h>

/**This class is used internally by TextToSpeechSystem in order to do the actual speaking.
  *@author Gunnar Schmi Dt
  */

class Speech : public TQObject {
   Q_OBJECT
public:
   enum CharacterCodec {
      Local    = 0,
      Latin1   = 1,
      Unicode  = 2,
      UseCodec = 3
   };

   Speech();
   ~Speech();

   /**
    * Speaks the given text.
    * @param command the program that shall be executed for speaking
    * @param stdin true if the program shall receive its data via standard input
    * @param text The text that shall be spoken
    */
   void speak(TQString command, bool use_stdin, const TQString &text, const TQString &language, int encoding, TQTextCodec *codec);

   /**
    * Prepares a command for being executed. During the preparation the
    * command is parsed and occurrences of "%t" are replaced by text.
    * @param command the command that shall be executed for speaking
    * @param text the quoted text that can be inserted into the command
    */
   TQString prepareCommand (TQString command, const TQString &text,
                     const TQString &filename, const TQString &language);

public slots:
   void wroteStdin (KProcess *p);
   void processExited (KProcess *p);
   void receivedStdout (KProcess *proc, char *buffer, int buflen);
   void receivedStderr (KProcess *proc, char *buffer, int buflen);

private:
   KShellProcess process;
   TQByteArray encText;
   KTempFile tempFile;
};

#endif
