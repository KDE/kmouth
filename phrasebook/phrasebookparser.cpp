/***************************************************************************
                          phrasebookparser.cpp  -  description
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

#include "phrasebookparser.h"

PhraseBookParser::PhraseBookParser() {
}

PhraseBookParser::~PhraseBookParser() {
}

bool PhraseBookParser::warning (const QXmlParseException &) {
   return false;
}

bool PhraseBookParser::error (const QXmlParseException &) {
   return false;
}

bool PhraseBookParser::fatalError (const QXmlParseException &) {
   return false;
}

QString PhraseBookParser::errorString() {
   return "";
}

bool PhraseBookParser::startDocument() {
   list.clear ();
   isInPhrase = false;
   level = 0;
   offset = 0;
   starting = true;
   return true;
}

bool PhraseBookParser::startElement (const QString &, const QString &,
                                     const QString &name,
                                     const QXmlAttributes &attributes)
{
   if (name == "phrase") {
      if (isInPhrase)
         return false;

      phrase.phrase = "";
      phrase.shortcut = attributes.value("shortcut");
      isInPhrase = true;
   }
   else if (name == "phrasebook") {
      if (isInPhrase)
         return false;

      phrase.phrase = attributes.value("name");
      phrase.shortcut = "";
      if ((phrase.phrase.isNull() || phrase.phrase.isEmpty()) && starting)
         offset = -1;
      else {
         list += PhraseBookEntry (phrase, level, false);
         level++;
      }
      starting = false;
   }
   return true;
}

bool PhraseBookParser::characters (const QString &ch) {
   phrase.phrase += ch;
   return true;
}

bool PhraseBookParser::ignorableWhitespace (const QString &ch) {
   phrase.phrase += ch;
   return true;
}

bool PhraseBookParser::endElement (const QString &, const QString &,
                                   const QString &name)
{
   if (name == "phrase") {
      list += PhraseBookEntry (phrase, level, true);
      isInPhrase = false;
   }
   else if (name == "phrasebook") {
      if (level == offset)
         return false;

      level--;
   }
   return true;
}

bool PhraseBookParser::endDocument() {
   return (level == offset);
}

PhraseBookEntryList PhraseBookParser::getPhraseList() {
   return list;
}

/*
 * $Log$
 * Revision 1.1  2002/11/11 21:25:44  gunnar
 * Moved the parts concerning phrase books into a static library
 *
 * Revision 1.2  2002/10/29 20:29:22  gunnar
 * Small changes
 *
 * Revision 1.1  2002/10/21 16:13:30  gunnar
 * phrase book format implemented
 *
 */
