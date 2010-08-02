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

#include <tqxml.h>
#include <tqvaluestack.h>
#include <tqstring.h>
#include <tqobject.h>
#include <tqmap.h>

class TQTextCodec;
class KProgressDialog;

namespace WordList {

typedef TQMap<TQString,int> WordMap;

KProgressDialog *progressDialog();

WordMap parseKDEDoc (TQString language, KProgressDialog *pdlg);
WordMap parseFile   (TQString filename,  TQTextStream::Encoding encoding, TQTextCodec *codec, KProgressDialog *pdlg);
WordMap parseDir    (TQString directory, TQTextStream::Encoding encoding, TQTextCodec *codec, KProgressDialog *pdlg);
WordMap mergeFiles  (TQMap<TQString,int> files, KProgressDialog *pdlg);

WordMap spellCheck  (WordMap wordlist,  TQString dictionary,   KProgressDialog *pdlg);

bool saveWordList (WordMap map, TQString filename);

/**
 * This class implements a parser for reading docbooks and generating word
 * lists. It is intended to be used together with the Qt SAX2 framework.
 * @author Gunnar Schmi Dt
 */

class XMLParser : public TQXmlDefaultHandler {
public: 
   XMLParser();
   ~XMLParser();

   bool warning (const TQXmlParseException &exception);
   bool error (const TQXmlParseException &exception);
   bool fatalError (const TQXmlParseException &exception);
   TQString errorString();
 
   /** Processes the start of the document. */
   bool startDocument();
                       
   /** Processes the start tag of an element. */
   bool startElement (const TQString &, const TQString &, const TQString &name,
                      const TQXmlAttributes &attributes);

   /** Processes a chunk of normal character data. */
   bool characters (const TQString &ch);

   /** Processes whitespace. */
   bool ignorableWhitespace (const TQString &ch);

   /** Processes the end tag of an element. */
   bool endElement (const TQString &, const TQString &, const TQString &name);
   
   /** Processes the end of the document. */
   bool endDocument();

   /** returns a list of words */
   WordMap getList();

private:
   WordMap list;
   TQString text;
};

}

#endif
