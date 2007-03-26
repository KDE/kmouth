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

#include <QObject>
#include <QString>
#include <k3process.h>
#include <ktemporaryfile.h>

/**This class is used internally by TextToSpeechSystem in order to do the actual speaking.
  *@author Gunnar Schmi Dt
  */

class Speech : public QObject {
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
   void speak(QString command, bool use_stdin, const QString &text, const QString &language, int encoding, QTextCodec *codec);

   /**
    * Prepares a command for being executed. During the preparation the
    * command is parsed and occurrences of "%t" are replaced by text.
    * @param command the command that shall be executed for speaking
    * @param text the quoted text that can be inserted into the command
    */
   QString prepareCommand (QString command, const QString &text,
                     const QString &filename, const QString &language);

public slots:
   void wroteStdin (K3Process *p);
   void processExited (K3Process *p);
   void receivedStdout (K3Process *proc, char *buffer, int buflen);
   void receivedStderr (K3Process *proc, char *buffer, int buflen);

private:
   K3ShellProcess process;
   QByteArray encText;
   KTemporaryFile tempFile;
};

#endif
