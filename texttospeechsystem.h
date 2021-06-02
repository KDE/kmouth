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

#ifndef TEXTTOSPEECHSYSTEM_H
#define TEXTTOSPEECHSYSTEM_H

#include <QList>
#include <QObject>

class QTextToSpeech;

/**This class represents a text-to-speech system.
  *@author Gunnar Schmi Dt
  */
class TextToSpeechSystem : public QObject
{
    Q_OBJECT
    friend class TextToSpeechConfigurationWidget;
public:
    explicit TextToSpeechSystem(QObject *parent = nullptr);
    ~TextToSpeechSystem() override;

    void readOptions(const QString &langGroup);
    void saveOptions(const QString &langGroup);

public Q_SLOTS:
    void speak(const QString &text, const QString &language);

private:
    void buildCodecList();

    QList<QTextCodec*> *codecList;
    int codec;
    QString ttsCommand;
    bool stdIn;
    bool useQtSpeech;
    /** Text to Speech API */
    QTextToSpeech *m_speech;
};

#endif
