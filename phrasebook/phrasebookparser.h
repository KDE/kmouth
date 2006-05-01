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

#include <qxml.h>
#include <q3valuestack.h>
#include "phrasebook.h"

/**
 * This class implements a parser for both the phrase list and for phrase
 * books. It is intended to be used together with the Qt SAX2 framework.
 * @author Gunnar Schmi Dt
 */

class PhraseBookParser : public QXmlDefaultHandler {
public: 
   PhraseBookParser();
   ~PhraseBookParser();

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
