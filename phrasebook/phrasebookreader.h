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

#ifndef PHRASEBOOKREADER_H
#define PHRASEBOOKREADER_H

#include <QXmlStreamReader>

#include "phrasebook.h"

/**
 * This class implements a reader for both the phrase list and for phrase
 * books. Pass a QIODevice to read check the return value and get the list.
 */

class PhraseBookReader
{
public:
    PhraseBookReader();
    ~PhraseBookReader();

    bool read(QIODevice *device);

    QString errorString() const;

    /** returns a list of phrase book entries */
    PhraseBookEntryList getPhraseList();

private:
    // Read an entire book
    void readbook();

    // Read a phrase
    void readphrase();

    PhraseBookEntryList list;
    QXmlStreamReader xml;

    int level;
    bool starting;
};

#endif
