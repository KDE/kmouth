/***************************************************************************
                          phrasebook.cpp  -  description
                             -------------------
    begin                : Don Sep 19 2002
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

#include <qprinter.h>
#include <qpainter.h>
#include <qfile.h>
#include <qxml.h>
#include <qregexp.h>
#include <qptrstack.h>

#include <klocale.h>
#include <kaction.h>
#include <kpopupmenu.h>
#include <ktoolbar.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>

#include "phrasebookparser.h"
#include "phrasebook.h"

Phrase::Phrase() {
   this->phrase = "";
   this->shortcut = "";
};

Phrase::Phrase (const QString &phrase) {
   this->phrase = phrase;
   this->shortcut = "";
};

Phrase::Phrase (const QString &phrase, const QString &shortcut) {
   this->phrase = phrase;
   this->shortcut = shortcut;
};

QString Phrase::getPhrase() const {
   return phrase;
};

QString Phrase::getShortcut() const {
   return shortcut;
};

void Phrase::setPhrase (const QString &phrase) {
   this->phrase = phrase;
};

void Phrase::setShortcut (const QString &shortcut) {
   this->shortcut = shortcut;
};

// ***************************************************************************

PhraseBookEntry::PhraseBookEntry () {
   phrase = Phrase();
   level = 1;
   isPhraseValue = false;
}

PhraseBookEntry::PhraseBookEntry (Phrase phrase, int level, bool isPhrase) {
   this->phrase = phrase;
   this->level = level;
   isPhraseValue = isPhrase;
}

bool PhraseBookEntry::isPhrase() const {
  return isPhraseValue;
}

Phrase PhraseBookEntry::getPhrase() const {
   return phrase;
}

int PhraseBookEntry::getLevel() const {
   return level;
}

// ***************************************************************************

void PhraseBook::print(KPrinter *pPrinter) {
   QPainter printpainter;
   printpainter.begin(pPrinter);

   QRect size = printpainter.viewport ();
   int x = size.x();
   int y = size.y();
   int w = size.width();
   printpainter.setFont (QFont ("Helvetica", 12));
   QFontMetrics metrics = printpainter.fontMetrics();

   PhraseBookEntryList::iterator it;
   for (it = begin(); it != end(); ++it) {
      QRect rect = metrics.boundingRect (x+16*(*it).getLevel(), y,
                                         w-16*(*it).getLevel(), 0,
                                         Qt::AlignJustify | Qt::WordBreak,
                                         (*it).getPhrase().getPhrase());

      if (y+rect.height() > size.height()) {
         pPrinter->newPage();
         y = 0;
      }
      printpainter.drawText (x+16*(*it).getLevel(),y,
                             w-16*(*it).getLevel(),rect.height(),
                             Qt::AlignJustify | Qt::WordBreak,
                             (*it).getPhrase().getPhrase());
      y += rect.height();
   }

   printpainter.end();
}

bool PhraseBook::decode (const QString &xml) {
   QXmlInputSource source;
   source.setData (xml);
   return decode (source);
}

bool PhraseBook::decode (QXmlInputSource &source) {
   PhraseBookParser parser;
   QXmlSimpleReader reader;
   reader.setFeature ("http://trolltech.com/xml/features/report-start-end-entity", true);
   reader.setContentHandler (&parser);

   if (reader.parse(source)) {
      PhraseBookEntryList::clear();
      *(PhraseBookEntryList *)this += parser.getPhraseList();
      return true;
   }
   else
      return false;
}

QCString encodeString (const QString str) {
   QCString res = "";
   for (int i = 0; i < (int)str.length(); i++) {
      QChar ch = str.at(i);
      ushort uc = ch.unicode();
      QCString number; number.setNum(uc);
      if ((uc>127) || (uc<32) || (ch=='<') || (ch=='>') || (ch=='&') || (ch==';'))
         res = res + "&#" + number + ";";
      else
         res = res + (char)uc;
   }
   return res;
}

QString PhraseBook::encode () {
   QString result;
   result  = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
   result += "<!DOCTYPE phrasebook>\n";
   result += "<phrasebook>\n";

   PhraseBookEntryList::iterator it;
   int level = 0;
   for (it = begin(); it != end(); ++it) {
      int newLevel = (*it).getLevel();
      while (level < newLevel) {
         result += "<phrasebook>\n";
         level++;
      }
      while (level > newLevel) {
         result += "</phrasebook>\n";
         level--;
      }

      if ((*it).isPhrase()) {
         Phrase phrase = (*it).getPhrase();
         result += "<phrase shortcut=\"" + encodeString(phrase.getShortcut());
         result += "\">" + encodeString(phrase.getPhrase()) + "</phrase>\n";
      }
      else {
         Phrase phrase = (*it).getPhrase();
         result += "<phrasebook name=\"" + encodeString(phrase.getPhrase()) + "\">\n";
         level++;
      }
   }
   while (level > 0) {
      result += "</phrasebook>\n";
      level--;
   }
   result += "</phrasebook>";
   return result;
}

QStringList PhraseBook::toStringList () {
   QStringList result;

   PhraseBook::iterator it;
   for (it = begin(); it != end(); ++it) {
      if ((*it).isPhrase())
         result += (*it).getPhrase().getPhrase();
   }
   return result;
}

bool PhraseBook::save (const KURL &url) {
   QRegExp pattern("*.phrasebook",true,true);
   return save (url, pattern.exactMatch(url.filename()));
}

bool PhraseBook::save (const KURL &url, bool asPhrasebook) {
   KTempFile tempFile;
   tempFile.setAutoDelete(true);

   if (asPhrasebook)
      *tempFile.textStream() << encode();
   else {
      *tempFile.textStream() << toStringList().join("\n");
   }
   tempFile.close();

   return KIO::NetAccess::upload(tempFile.name(), url);
}

int PhraseBook::save (QWidget *parent, const QString &title, KURL &url, bool phrasebookFirst) {
   // KFileDialog::getSaveURL(...) is not usefull here as we need
   // to know the requested file type.

   QString filters;
   if (phrasebookFirst)
      filters = i18n("*.phrasebook|Phrase Books (*.phrasebook)\n*.txt|Plain Text Files (*.txt)\n*|All Files");
   else
      filters = i18n("*.txt|Plain Text Files (*.txt)\n*.phrasebook|Phrase Books (*.phrasebook)\n*|All Files");

   KFileDialog fdlg(QString::null,filters, parent, "filedialog", true);
   fdlg.setCaption(title);
   fdlg.setOperationMode( KFileDialog::Saving );

   fdlg.exec();
   url = fdlg.selectedURL();

   if(!url.isEmpty()) {
      bool result;
      if (fdlg.currentFilter() == "*.phrasebook")
         result = save (url, true);
      else if (fdlg.currentFilter() == "*.txt")
         result = save (url, false);
      else // file format "All files" requested, so decide by extension
         result = save (url);

      if (result)
         return 1;
      else
         return -1;
   }
   return 0;
}

bool PhraseBook::open (const KURL &url) {
   QString tempFile;
   KURL fileUrl = url;

   QString protocol = fileUrl.protocol();
   if (protocol.isEmpty() || protocol.isNull()) {
      fileUrl.setProtocol ("file");
      fileUrl.setPath (url.url());
   }

   if (KIO::NetAccess::download (fileUrl, tempFile)) {
      QStringList list = QStringList();

      // First: try to load it as a normal phrase book
      QFile file(tempFile);
      QXmlInputSource source (&file);
      bool error = !decode (source);

      // Second: if the file does not contain a phrase book, load it as
      // a plain text file
      if (error) {
         // Load each line of the plain text file as a new phrase

         QFile file(tempFile);
         if (file.open(IO_ReadOnly)) {
            QTextStream stream(&file);

            while (!stream.atEnd()) {
               QString s = stream.readLine();
               if (!(s.isNull() || s.isEmpty()))
                  *this += PhraseBookEntry(Phrase(s, ""), 0, true);
            }
            file.close();
            error = false;
         }
         else
            error = true;
      }
      KIO::NetAccess::removeTempFile (tempFile);

      return !error;
   }
   else
      return false;
}

void PhraseBook::addToGUI (QPopupMenu *popup, KToolBar *toolbar, KActionCollection *phrases,
                  QObject *receiver, const char *slot) const {
   if ((popup != 0) || (toolbar != 0)) {
      QPtrStack<QWidget> stack;
      QWidget *parent = popup;
      int level = 0;

      QValueListConstIterator<PhraseBookEntry> it;
      for (it = begin(); it != end(); ++it) {
         int newLevel = (*it).getLevel();
         while (newLevel > level) {
            KActionMenu *menu = new KActionMenu("", "phrasebook");
            menu->setDelayed(false);
            phrases->insert(menu);
            menu->plug (parent);
            if (parent == popup)
               menu->plug (toolbar);
            if (parent != 0)
               stack.push (parent);
            parent = menu->popupMenu();
            level++;
         }
         while (newLevel < level && (parent != popup)) {
            parent = stack.pop();
            level--;
         }
         if ((*it).isPhrase()) {
            Phrase phrase = (*it).getPhrase();
            KAction *action = new PhraseAction (phrase.getPhrase(),
                     phrase.getShortcut(), receiver, slot, phrases);
            if (parent == popup)
               action->plug (toolbar);
            if (parent != 0)
               action->plug(parent);
         }
         else {
            Phrase phrase = (*it).getPhrase();
            KActionMenu *menu = new KActionMenu(phrase.getPhrase(), "phrasebook");
            menu->setDelayed(false);
            phrases->insert(menu);
            if (parent == popup)
               menu->plug (toolbar);
            if (parent != 0)
               menu->plug (parent);
            stack.push (parent);
            parent = menu->popupMenu();
            level++;
         }
      }
   }
}

void PhraseBook::insert (const QString &name, const PhraseBook &book) {
   *this += PhraseBookEntry(Phrase(name), 0, false);

   QValueListConstIterator<PhraseBookEntry> it;
   for (it = book.begin(); it != book.end(); ++it) {
      *this += PhraseBookEntry ((*it).getPhrase(), (*it).getLevel()+1, (*it).isPhrase());
   }
}

// ***************************************************************************

PhraseBookDrag::PhraseBookDrag (PhraseBook *book, QWidget *dragSource, const char *name)
    : QDragObject (dragSource, name)
{
   setBook (book);
}

PhraseBookDrag::PhraseBookDrag (QWidget *dragSource, const char *name)
    : QDragObject (dragSource, name)
{
   setBook (0);
}

PhraseBookDrag::~PhraseBookDrag () {
}

void PhraseBookDrag::setBook (PhraseBook *book) {
   if (book == 0) {
      isEmpty = true;
      xmlphrasebook.setText(QString::null);
      xml.setText(QString::null);
      plain.setText(QString::null);
   }
   else {
      isEmpty = false;
      xmlphrasebook.setText(book->encode());
      xml.setText(book->encode());
      plain.setText(book->toStringList().join("\n"));
   }
   xmlphrasebook.setSubtype("x-xml-phrasebook");
   xml.setSubtype("xml");
   plain.setSubtype("plain");
}

const char *PhraseBookDrag::format (int i) const {
   if (isEmpty)
      return plain.format(i);
   else if (i%3 == 0)
      return plain.format(i/3);
   else if (i%3 == 1)
      return xml.format(i/3);
   else
      return xmlphrasebook.format(i/3);
}

QByteArray PhraseBookDrag::encodedData (const char* mime) const {
   QCString m(mime);
   m = m.lower();
   if (m.contains("xml-phrasebook"))
      return xmlphrasebook.encodedData(mime);
   else if (m.contains("xml"))
      return xml.encodedData(mime);
   else
      return plain.encodedData(mime);
}

bool PhraseBookDrag::canDecode (const QMimeSource* e) {
   return QTextDrag::canDecode(e);
}

bool PhraseBookDrag::decode (const QMimeSource *e, PhraseBook *book) {
   QString string;
   QCString subtype1 = "x-xml-phrasebook";
   QCString subtype2 = "xml";

   if (!QTextDrag::decode(e, string, subtype1))
      if (!QTextDrag::decode(e, string, subtype2)) {
         if (QTextDrag::decode(e, string)) {
            *book += PhraseBookEntry(Phrase(string, ""), 0, true);
            return true;
         }
         else return false;
      }

   return book->decode(string);
}

#include "phrasebook.moc"

/*
 * $Log$
 * Revision 1.2  2003/01/18 07:29:12  binner
 * CVS_SILENT i18n style guide fixes
 *
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.6  2003/01/12 21:52:50  gunnar
 * Printing improved.
 *
 * Revision 1.5  2003/01/12 20:26:02  gunnar
 * Printing phrase book added
 *
 * Revision 1.4  2003/01/12 11:37:05  gunnar
 * Improved format list of file selectors / several small changes
 *
 * Revision 1.3  2002/12/06 08:55:05  gunnar
 * Improved the algorithm for creating the initial phrasebook
 *
 * Revision 1.2  2002/12/04 16:22:02  gunnar
 * Include *.moc files
 *
 * Revision 1.1  2002/11/11 21:25:44  gunnar
 * Moved the parts concerning phrase books into a static library
 *
 * Revision 1.6  2002/10/29 20:29:22  gunnar
 * Small changes
 *
 * Revision 1.5  2002/10/29 18:11:17  gunnar
 * Implemented opening and saving of a standard phrasebook
 *
 * Revision 1.4  2002/10/28 16:58:34  gunnar
 * Import and export of phrase books implemented
 *
 * Revision 1.3  2002/10/23 22:19:30  gunnar
 * Cut, copy and paste features of the phrase book edit dialog improved
 *
 * Revision 1.2  2002/10/22 20:10:29  gunnar
 * Cut and copy of phrase book entries implemented
 *
 * Revision 1.1  2002/10/21 16:13:30  gunnar
 * phrase book format implemented
 *
 */
