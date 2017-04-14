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

#include "texttospeechsystem.h"

#include <QTextCodec>
#include <QTextToSpeech>

#include <KSharedConfig>
#include <KConfigGroup>

#include "speech.h"

TextToSpeechSystem::TextToSpeechSystem()
{
    stdIn = true;
    useQtSpeech = true;
    codec = Speech::Local; // local encoding;
    buildCodecList();
    m_speech = new QTextToSpeech();
}

TextToSpeechSystem::~TextToSpeechSystem()
{
    delete codecList;
    delete m_speech;
}

void TextToSpeechSystem::speak(const QString &text, const QString &language)
{
    if (text.length() > 0) {
        if (useQtSpeech) {
            m_speech->say(text);
            return;
        }

        if (codec < Speech::UseCodec)
            (new Speech())->speak(ttsCommand, stdIn, text, language, codec, 0);
        else
            (new Speech())->speak(ttsCommand, stdIn, text, language, Speech::UseCodec,
                                  codecList->at(codec - Speech::UseCodec));
    }
}

void TextToSpeechSystem::readOptions(const QString &langGroup)
{
    KConfigGroup cg(KSharedConfig::openConfig(), langGroup);
    ttsCommand = cg.readPathEntry("Command", QString());
    stdIn = cg.readEntry("StdIn", true);
    useQtSpeech = cg.readEntry("useQtSpeech", true);

    QString codecString = cg.readEntry("Codec", "Local");
    if (codecString == QLatin1String("Local"))
        codec = Speech::Local;
    else if (codecString == QLatin1String("Latin1"))
        codec = Speech::Latin1;
    else if (codecString == QLatin1String("Unicode"))
        codec = Speech::Unicode;
    else {
        codec = Speech::Local;
        for (int i = 0; i < codecList->count(); i++)
            if (codecString == QLatin1String(codecList->at(i)->name()))
                codec = Speech::UseCodec + i;
    }
}

void TextToSpeechSystem::saveOptions(const QString &langGroup)
{
    KConfigGroup cg(KSharedConfig::openConfig(), langGroup);
    cg.writePathEntry("Command", ttsCommand);
    cg.writeEntry("StdIn", stdIn);
    cg.writeEntry("useQtSpeech", useQtSpeech);
    if (codec == Speech::Local)
        cg.writeEntry("Codec", "Local");
    else if (codec == Speech::Latin1)
        cg.writeEntry("Codec", "Latin1");
    else if (codec == Speech::Unicode)
        cg.writeEntry("Codec", "Unicode");
    else {
        QString codeName = QLatin1String(codecList->at(codec - Speech::UseCodec)->name());
        cg.writeEntry("Codec", codeName);
    }
}

void TextToSpeechSystem::buildCodecList()
{
    codecList = new QList<QTextCodec*>;
    QList<QByteArray> availableCodecs = QTextCodec::availableCodecs();
    for (int i = 0; i < availableCodecs.count(); ++i) {
        QTextCodec *codec = QTextCodec::codecForName(availableCodecs[i]);
        codecList->append(codec);
    }
}

#include "texttospeechsystem.moc"
