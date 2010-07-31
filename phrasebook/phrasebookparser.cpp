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

#include "phrasebookparser.h"

PhraseBookParser::PhraseBookParser() {
}

PhraseBookParser::~PhraseBookParser() {
}

bool PhraseBookParser::warning (const TQXmlParseException &) {
   return false;
}

bool PhraseBookParser::error (const TQXmlParseException &) {
   return false;
}

bool PhraseBookParser::fatalError (const TQXmlParseException &) {
   return false;
}

TQString PhraseBookParser::errorString() {
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

bool PhraseBookParser::startElement (const TQString &, const TQString &,
                                     const TQString &name,
                                     const TQXmlAttributes &attributes)
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

bool PhraseBookParser::characters (const TQString &ch) {
   phrase.phrase += ch;
   return true;
}

bool PhraseBookParser::ignorableWhitespace (const TQString &ch) {
   phrase.phrase += ch;
   return true;
}

bool PhraseBookParser::endElement (const TQString &, const TQString &,
                                   const TQString &name)
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
