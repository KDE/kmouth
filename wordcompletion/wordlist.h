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

#ifndef WORDLIST_H
#define WORDLIST_H

#include <QMap>
#include <QXmlStreamReader>

class QTextCodec;
class QProgressDialog;

namespace WordList
{

typedef QMap<QString, int> WordMap;

QProgressDialog *progressDialog();

WordMap parseKDEDoc(QString language, QProgressDialog *pdlg);
WordMap parseFile(const QString &filename, QTextCodec *codec, QProgressDialog *pdlg);
WordMap parseDir(const QString &directory, QTextCodec *codec, QProgressDialog *pdlg);
WordMap mergeFiles(const QMap<QString, int> &files, QProgressDialog *pdlg);

WordMap spellCheck(WordMap wordlist,  const QString &dictionary,   QProgressDialog *pdlg);

bool saveWordList(const WordMap &map, const QString &filename);

/**
 * This class implements a parser for reading docbooks and generating word
 * lists.
 */

class XMLReader
{
public:
    XMLReader();
    ~XMLReader();

    bool read(QIODevice *device);

    QString errorString() const;

    /** returns a list of words */
    WordMap getList();

private:
    WordMap list;
    QString text;

    QXmlStreamReader xml;
};

}

#endif
