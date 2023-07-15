/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015,2022 by Jeremy Whiting <jpwhiting@kde.org>         *
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

#include "phrasebookreader.h"

#include <QtDebug>

PhraseBookReader::PhraseBookReader()
    : level(0)
    , starting(true)
{
}

PhraseBookReader::~PhraseBookReader()
{
}

bool PhraseBookReader::read(QIODevice *device)
{
    xml.setDevice(device);

    list.clear();
    level = 0;
    starting = true;

    qDebug() << "Beginning to read xml document";

    if (xml.readNextStartElement()) {
        qDebug() << "First node name " << xml.name();
        if (xml.name() == QLatin1String("phrasebook")) {
            readbook();
        } else if (xml.name() == QLatin1String("phrase")) {
            readphrase();
        } else {
            xml.raiseError(QLatin1String("The file is not a valid phrasebook xml file"));
        }
    } else {
        xml.raiseError(QLatin1String("Unable to get first xml element"));
    }

    return !xml.error();
}

void PhraseBookReader::readbook()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("phrasebook"));

    // Get the book's name attribute if any
    QString name = xml.attributes().value(QLatin1String("name")).toString();

    qDebug() << "Reading book with name " << name << " level is " << level;
    Phrase phrase;
    phrase.setPhrase(name);

    if ((phrase.getPhrase().isNull() || phrase.getPhrase().isEmpty()) && starting) {
        // Phrase book name is empty, and starting, so don't increase level or add empty entry
    } else {
        list += PhraseBookEntry(phrase, level, false);
        level++;
    }
    starting = false;

    while (xml.readNextStartElement()) {
        if (xml.name() == QLatin1String("phrase")) {
            readphrase();
        } else if (xml.name() == QLatin1String("phrasebook")) {
            readbook();
        } else
            xml.skipCurrentElement();
    }

    level--;
}

void PhraseBookReader::readphrase()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == QLatin1String("phrase"));

    QString shortcut = xml.attributes().value(QLatin1String("shortcut")).toString();

    Phrase phrase;
    phrase.setShortcut(shortcut);
    phrase.setPhrase(xml.readElementText());

    qDebug() << "Reading phrase with text " << phrase.getPhrase();

    list += PhraseBookEntry(phrase, level, true);
}

QString PhraseBookReader::errorString() const
{
    return QLatin1String("%1\nLine %2, column %3").arg(xml.errorString()).arg(xml.lineNumber()).arg(xml.columnNumber());
}

PhraseBookEntryList PhraseBookReader::getPhraseList()
{
    return list;
}
