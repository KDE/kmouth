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

#include "phrasebookparser.h"

PhraseBookParser::PhraseBookParser()
{
}

PhraseBookParser::~PhraseBookParser()
{
}

bool PhraseBookParser::warning(const QXmlParseException &)
{
    return false;
}

bool PhraseBookParser::error(const QXmlParseException &)
{
    return false;
}

bool PhraseBookParser::fatalError(const QXmlParseException &)
{
    return false;
}

QString PhraseBookParser::errorString() const
{
    return QLatin1String("");
}

bool PhraseBookParser::startDocument()
{
    list.clear();
    isInPhrase = false;
    level = 0;
    offset = 0;
    starting = true;
    return true;
}

bool PhraseBookParser::startElement(const QString &, const QString &,
                                    const QString &name,
                                    const QXmlAttributes &attributes)
{
    if (name == QLatin1String("phrase")) {
        if (isInPhrase)
            return false;

        phrase.phrase.clear();
        phrase.shortcut = attributes.value(QLatin1String("shortcut"));
        isInPhrase = true;
    } else if (name == QLatin1String("phrasebook")) {
        if (isInPhrase)
            return false;

        phrase.phrase = attributes.value(QLatin1String("name"));
        phrase.shortcut.clear();
        if ((phrase.phrase.isNull() || phrase.phrase.isEmpty()) && starting)
            offset = -1;
        else {
            list += PhraseBookEntry(phrase, level, false);
            level++;
        }
        starting = false;
    }
    return true;
}

bool PhraseBookParser::characters(const QString &ch)
{
    phrase.phrase += ch;
    return true;
}

bool PhraseBookParser::ignorableWhitespace(const QString &ch)
{
    phrase.phrase += ch;
    return true;
}

bool PhraseBookParser::endElement(const QString &, const QString &,
                                  const QString &name)
{
    if (name == QLatin1String("phrase")) {
        list += PhraseBookEntry(phrase, level, true);
        isInPhrase = false;
    } else if (name == QLatin1String("phrasebook")) {
        if (level == offset)
            return false;

        level--;
    }
    return true;
}

bool PhraseBookParser::endDocument()
{
    return (level == offset);
}

PhraseBookEntryList PhraseBookParser::getPhraseList()
{
    return list;
}
