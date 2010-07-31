/***************************************************************************
                          phrasebookparser.h  -  description
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

#ifndef PHRASEBOOKPARSER_H
#define PHRASEBOOKPARSER_H

#include <tqxml.h>
#include <tqvaluestack.h>
#include "phrasebook.h"

/**
 * This class implements a parser for both the phrase list and for phrase
 * books. It is intended to be used together with the Qt SAX2 framework.
 * @author Gunnar Schmi Dt
 */

class PhraseBookParser : public TQXmlDefaultHandler {
public: 
   PhraseBookParser();
   ~PhraseBookParser();

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

   /** returns a list of phrase book entries */
   PhraseBookEntryList getPhraseList();

private:
   bool isInPhrase;
   bool starting;
   int offset;
   Phrase phrase;
   
   PhraseBookEntryList list;
   int level;
};

#endif
