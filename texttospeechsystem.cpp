/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015, 2022 by Jeremy Whiting <jpwhiting@kde.org>        *
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

#include <QTextToSpeech>

#include <KConfigGroup>
#include <KSharedConfig>

#include "speech.h"

TextToSpeechSystem::TextToSpeechSystem(QObject *parent)
    : QObject(parent)
    , encoding(QStringConverter::System)
    , stdIn(true)
    , useQtSpeech(true)
    , ttsEngine(QLatin1String("speechd"))
    , m_speech(new QTextToSpeech(ttsEngine))
{
}

TextToSpeechSystem::~TextToSpeechSystem()
{
    delete m_speech;
}

void TextToSpeechSystem::speak(const QString &text, const QString &language)
{
    if (!text.isEmpty()) {
        if (useQtSpeech) {
            m_speech->say(text);
            return;
        }

        (new Speech())->speak(ttsCommand, stdIn, text, language, QStringConverter::Encoding(encoding));
    }
}

void TextToSpeechSystem::readOptions(const QString &langGroup)
{
    KConfigGroup cg(KSharedConfig::openConfig(), langGroup);
    ttsCommand = cg.readPathEntry("Command", QString());
    stdIn = cg.readEntry("StdIn", true);
    useQtSpeech = cg.readEntry("useQtSpeech", true);
    ttsEngine = cg.readEntry("ttsEngine", "speechd");
    // No default, depends on current locale, etc. so just naturally
    // select first voice if none set by user.
    ttsVoice = cg.readEntry("ttsVoice", "");

    // Get encoding from settings, but default to system encoding, which
    // is what "Local" used to be
    encoding = QStringConverter::Encoding(cg.readEntry("encoding", int(QStringConverter::System)));
}

void TextToSpeechSystem::saveOptions(const QString &langGroup)
{
    KConfigGroup cg(KSharedConfig::openConfig(), langGroup);
    cg.writePathEntry("Command", ttsCommand);
    cg.writeEntry("StdIn", stdIn);
    cg.writeEntry("useQtSpeech", useQtSpeech);
    cg.writeEntry("ttsEngine", ttsEngine);
    cg.writeEntry("ttsVoice", ttsVoice);
    cg.writeEntry("encoding", encoding);

    delete m_speech;
    m_speech = new QTextToSpeech(ttsEngine);
    const QVector<QVoice> voices = m_speech->availableVoices();
    for (const QVoice &voice : voices) {
        if (voice.name() == ttsVoice) {
            m_speech->setVoice(voice);
        }
    }
}
