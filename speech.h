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

// $Id$

#ifndef SPEECH_H
#define SPEECH_H

#include <qobject.h>
#include <qstring.h>
#include <kprocess.h>

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
    * @param stdin true if the program shall recieve its data via standard input
    * @param text The text that shall be spoken
    */
   void speak(QString command, bool use_stdin, QString text, int encoding, QTextCodec *codec);

   /**
    * Prepares a command for being executed. During the preparation the
    * command is parsed and occurences of "%t" are replaced by text.
    * @param command the command that shall be executed for speaking
    * @param text the quoted text that can be inserted into the command
    */
   QString prepareCommand (QString command, QString text);

public slots:
   void wroteStdin (KProcess *p);
   void processExited (KProcess *p);
   void receivedStdout (KProcess *proc, char *buffer, int buflen);
   void receivedStderr (KProcess *proc, char *buffer, int buflen);

private:
   KShellProcess process;
   QByteArray encText;
};

#endif

/*
 * $Log$
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.3  2002/12/11 18:57:31  gunnar
 * Incorporated changes for version 0.5.2 into head branch
 *
 * Revision 1.1.2.2  2002/12/11 18:31:23  gunnar
 * Security fix
 *
 * Revision 1.2  2002/11/04 16:38:42  gunnar
 * Incorporated changes for version 0.5.1 into head branch
 *
 * Revision 1.1.2.1  2002/11/04 15:36:37  gunnar
 * combo box for character encoding added
 *
 * Revision 1.1  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 */
