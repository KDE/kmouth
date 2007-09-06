/***************************************************************************
                          wordlist.h  -  description
                             -------------------
    begin                : Don Sep 12 2002
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

#ifndef WORDLIST_H
#define WORDLIST_H

#include <QtXml>
#include <Qt3Support/Q3ValueStack>
#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QTextStream>

class QTextCodec;
class KProgressDialog;

namespace WordList {

typedef QMap<QString,int> WordMap;

KProgressDialog *progressDialog();

WordMap parseKDEDoc (QString language, KProgressDialog *pdlg);
WordMap parseFile   (QString filename,  QTextStream::Encoding encoding, QTextCodec *codec, KProgressDialog *pdlg);
WordMap parseDir    (QString directory, QTextStream::Encoding encoding, QTextCodec *codec, KProgressDialog *pdlg);
WordMap mergeFiles  (QMap<QString,int> files, KProgressDialog *pdlg);

WordMap spellCheck  (WordMap wordlist,  QString dictionary,   KProgressDialog *pdlg);

bool saveWordList (WordMap map, QString filename);

/**
 * This class implements a parser for reading docbooks and generating word
 * lists. It is intended to be used together with the Qt SAX2 framework.
 * @author Gunnar Schmi Dt
 */

class XMLParser : public QXmlDefaultHandler {
public: 
   XMLParser();
   ~XMLParser();

   bool warning (const QXmlParseException &exception);
   bool error (const QXmlParseException &exception);
   bool fatalError (const QXmlParseException &exception);
   QString errorString() const;
 
   /** Processes the start of the document. */
   bool startDocument();
                       
   /** Processes the start tag of an element. */
   bool startElement (const QString &, const QString &, const QString &name,
                      const QXmlAttributes &attributes);

   /** Processes a chunk of normal character data. */
   bool characters (const QString &ch);

   /** Processes whitespace. */
   bool ignorableWhitespace (const QString &ch);

   /** Processes the end tag of an element. */
   bool endElement (const QString &, const QString &, const QString &name);
   
   /** Processes the end of the document. */
   bool endDocument();

   /** returns a list of words */
   WordMap getList();

private:
   WordMap list;
   QString text;
};

}

#endif
