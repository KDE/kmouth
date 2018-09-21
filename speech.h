/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015 by Jeremy Whiting <jpwhiting@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef SPEECH_H
#define SPEECH_H

#include <QObject>
#include <QProcess>
#include <QTemporaryFile>

/**This class is used internally by TextToSpeechSystem in order to do the actual speaking.
  *@author Gunnar Schmi Dt
  */

class Speech : public QObject
{
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
    QString prepareCommand(QString command, const QString &text,
                           const QString &filename, const QString &language);

public Q_SLOTS:
    //void wroteStdin(K3Process *p);
    void processExited(int exitCode, QProcess::ExitStatus exitStatus);
    //void receivedStdout(K3Process *proc, char *buffer, int buflen);
    //void receivedStderr(K3Process *proc, char *buffer, int buflen);

private:
    QProcess m_process;
    QByteArray encText;
    QTemporaryFile tempFile;
};

#endif
