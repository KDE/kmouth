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

#include "speech.h"

#include <QHash>
#include <QStack>
#include <QTextCodec>
#include <QTextStream>

#define macroExpander
#include <KMacroExpander>

Speech::Speech()
{
}

Speech::~Speech()
{
}

QString Speech::prepareCommand(const QString &command, const QString &text,
                               const QString &filename, const QString &language)
{
    QHash<QChar, QString> map;
    map[QLatin1Char('t')] = text;
    map[QLatin1Char('f')] = filename;
    map[QLatin1Char('l')] = language;
    return KMacroExpander::expandMacrosShellQuote(command, map);
}

void Speech::speak(QString command, bool stdIn, const QString &text, const QString &language, int encoding, QTextCodec *codec)
{
    if (text.length() > 0) {
        // 1. prepare the text:
        // 1.a) encode the text
        QTextStream ts(&encText, QIODevice::WriteOnly);
        if (encoding == Local)
            ts.setCodec(QTextCodec::codecForLocale());
        else if (encoding == Latin1)
            ts.setCodec("ISO-8859-1");
        else if (encoding == Unicode)
            ts.setCodec("UTF-16");
        else
            ts.setCodec(codec);
        ts << text;
        ts.flush();

        // 1.b) create a temporary file for the text
        tempFile.open();
        QTextStream fs(&tempFile);
        if (encoding == Local)
            fs.setCodec(QTextCodec::codecForLocale());
        else if (encoding == Latin1)
            fs.setCodec("ISO-8859-1");
        else if (encoding == Unicode)
            fs.setCodec("UTF-16");
        else
            fs.setCodec(codec);
        fs << text;
        fs << "\n";
        QString filename = tempFile.fileName();
        tempFile.flush();

        // 2. prepare the command:
        command = prepareCommand(command, QLatin1String(encText), filename, language);

        // 3. create a new process
        connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Speech::processExited);
        //connect(&process, SIGNAL(wroteStdin(K3Process*)), this, SLOT(wroteStdin(K3Process*)));
        //connect(&process, SIGNAL(receivedStdout(K3Process*, char*, int)), this, SLOT(receivedStdout(K3Process*, char*, int)));
        //connect(&process, SIGNAL(receivedStderr(K3Process*, char*, int)), this, SLOT(receivedStderr(K3Process*, char*, int)));

        // 4. start the process
        if (stdIn) {
            m_process.start(command);
            if (!encText.isEmpty())
                m_process.write(encText.constData(), encText.size());
            else
                m_process.close();
        } else
            m_process.start(command);
    }
}

//void Speech::receivedStdout(K3Process *, char *buffer, int buflen)
//{
//    kDebug() << QString::fromLatin1(buffer, buflen) + QLatin1Char('\n');
//}
//void Speech::receivedStderr(K3Process *, char *buffer, int buflen)
//{
//    kDebug() << QString::fromLatin1(buffer, buflen) + QLatin1Char('\n');
//}

//void Speech::wroteStdin(K3Process *)
//{
//    process.closeStdin();
//}

void Speech::processExited(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
    delete this;
}

