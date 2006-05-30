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

#include <QPrinter>
#include <QPainter>
#include <QFile>
#include <QtXml>
#include <QRegExp>
#include <qstack.h>
#include <QTextStream>
#include <Q3PopupMenu>
#include <kactionmenu.h>
#include <kactioncollection.h>
#include <klocale.h>
#include <kaction.h>
#include <kmenu.h>
#include <ktoolbar.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kglobalsettings.h>

#include "phrasebookparser.h"
#include "phrasebook.h"

Phrase::Phrase() {
   this->phrase = "";
   this->shortcut = "";
}

Phrase::Phrase (const QString &phrase) {
   this->phrase = phrase;
   this->shortcut = "";
}

Phrase::Phrase (const QString &phrase, const QString &shortcut) {
   this->phrase = phrase;
   this->shortcut = shortcut;
}

QString Phrase::getPhrase() const {
   return phrase;
}

QString Phrase::getShortcut() const {
   return shortcut;
}

void Phrase::setPhrase (const QString &phrase) {
   this->phrase = phrase;
}

void Phrase::setShortcut (const QString &shortcut) {
   this->shortcut = shortcut;
}

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
   printpainter.setFont (QFont (KGlobalSettings::generalFont().family(), 12));
   QFontMetrics metrics = printpainter.fontMetrics();

   PhraseBookEntryList::iterator it;
   for (it = begin(); it != end(); ++it) {
      QRect rect = metrics.boundingRect (x+16*(*it).getLevel(), y,
                                         w-16*(*it).getLevel(), 0,
                                         Qt::AlignJustify | Qt::TextWordWrap,
                                         (*it).getPhrase().getPhrase());

      if (y+rect.height() > size.height()) {
         pPrinter->newPage();
         y = 0;
      }
      printpainter.drawText (x+16*(*it).getLevel(),y,
                             w-16*(*it).getLevel(),rect.height(),
                             Qt::AlignJustify | Qt::TextWordWrap,
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

QByteArray encodeString (const QString str) {
   QByteArray res = "";
   for (int i = 0; i < (int)str.length(); i++) {
      QChar ch = str.at(i);
      ushort uc = ch.unicode();
      QByteArray number; number.setNum(uc);
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

bool PhraseBook::save (const KUrl &url) {
   QRegExp pattern("*.phrasebook", Qt::CaseSensitive, QRegExp::Wildcard);
   return save (url, pattern.exactMatch(url.fileName()));
}


void PhraseBook::save (QTextStream &stream, bool asPhrasebook) {
   if (asPhrasebook)
      stream << encode();
   else
      stream << toStringList().join("\n");
}

bool PhraseBook::save (const KUrl &url, bool asPhrasebook) {
   if (url.isLocalFile()) {
      QFile file(url.path());
      if(!file.open(QIODevice::WriteOnly))
         return false;

      QTextStream stream(&file);
      save (stream, asPhrasebook);
      file.close();

      if (file.error() != QFile::NoError)
         return false;
      else
         return true;
   }
   else {
      KTempFile tempFile;
      tempFile.setAutoDelete(true);

      save (*tempFile.textStream(), asPhrasebook);
      tempFile.close();

      return KIO::NetAccess::upload(tempFile.name(), url,0L);
   }
}

int PhraseBook::save (QWidget *parent, const QString &title, KUrl &url, bool phrasebookFirst) {
   // KFileDialog::getSaveURL(...) is not useful here as we need
   // to know the requested file type.

   QString filters;
   if (phrasebookFirst)
      filters = i18n("*.phrasebook|Phrase Books (*.phrasebook)\n*.txt|Plain Text Files (*.txt)\n*|All Files");
   else
      filters = i18n("*.txt|Plain Text Files (*.txt)\n*.phrasebook|Phrase Books (*.phrasebook)\n*|All Files");

   KFileDialog fdlg(QString(),filters, parent);
   fdlg.setCaption(title);
   fdlg.setOperationMode( KFileDialog::Saving );

   if (fdlg.exec() != QDialog::Accepted) {
     return 0;
   }

   url = fdlg.selectedURL();

   if (url.isEmpty() || !url.isValid()) {
      return -1;
   }

   if (KIO::NetAccess::exists(url,false,0L)) {
      if (KMessageBox::warningContinueCancel(0,QString("<qt>%1</qt>").arg(i18n("The file %1 already exists. "
                                                       "Do you want to overwrite it?", url.url())),i18n("File Exists"),i18n("&Overwrite"))==KMessageBox::Cancel) {
         return 0;
      }
   }

   bool result;
   if (fdlg.currentFilter() == "*.phrasebook") {
      if (url.fileName (false).contains('.') == 0) {
         url.setFileName (url.fileName(false) + ".phrasebook");
      }
      else if (url.fileName (false).right (11).contains (".phrasebook", Qt::CaseInsensitive) == 0) {
         int filetype = KMessageBox::questionYesNoCancel (0,QString("<qt>%1</qt>").arg(i18n("Your chosen filename <i>%1</i> has a different extension than <i>.phrasebook</i>. "
                                                           "Do you wish to add <i>.phrasebook</i> to the filename?", url.fileName())),i18n("File Extension"),i18n("Add"),i18n("Do Not Add"));
         if (filetype == KMessageBox::Cancel) {
            return 0;
         }
         if (filetype == KMessageBox::Yes) {
            url.setFileName (url.fileName(false) + ".phrasebook");
         }
      }
      result = save (url, true);
   }
   else if (fdlg.currentFilter() == "*.txt") {
      if (url.fileName (false).right (11).contains (".phrasebook", Qt::CaseInsensitive) == 0) {
         result = save (url, false);
      }
      else {
         int filetype = KMessageBox::questionYesNoCancel (0,QString("<qt>%1</qt>").arg(i18n("Your chosen filename <i>%1</i> has the extension <i>.phrasebook</i>. "
                                                           "Do you wish to save in phrasebook format?", url.fileName())),i18n("File Extension"),i18n("As Phrasebook"),i18n("As Plain Text"));
         if (filetype == KMessageBox::Cancel) {
            return 0;
         }
         if (filetype == KMessageBox::Yes) {
            result = save (url, true);
         }
         else {
            result = save (url, false);
         }
      }
   }
   else // file format "All files" requested, so decide by extension
      result = save (url);

   if (result)
      return 1;
   else
      return -1;
}

bool PhraseBook::open (const KUrl &url) {
   QString tempFile;
   KUrl fileUrl = url;

   QString protocol = fileUrl.protocol();
   if (protocol.isEmpty() || protocol.isNull()) {
      fileUrl.setProtocol ("file");
      fileUrl.setPath (url.url());
   }

   if (KIO::NetAccess::download (fileUrl, tempFile,0L)) {
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
         if (file.open(QIODevice::ReadOnly)) {
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
      return false;
}

void PhraseBook::addToGUI (QMenu *popup, KToolBar *toolbar, KActionCollection *phrases,
                  QObject *receiver, const char *slot) const {
   if ((popup != 0) || (toolbar != 0)) {
      QStack<QWidget*> stack;
      QWidget *parent = popup;
      int level = 0;

      QList<PhraseBookEntry>::ConstIterator it;
      for (it = begin(); it != end(); ++it) {
         int newLevel = (*it).getLevel();
         while (newLevel > level) {
            KActionMenu *menu = new KActionMenu("", phrases,"phrasebook");
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
            KActionMenu *menu = new KActionMenu(phrase.getPhrase(),phrases, "phrasebook");
            menu->setDelayed(false);
            phrases->insert(menu);
            if (parent == popup)
               menu->plug (toolbar);
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

   QList<PhraseBookEntry>::ConstIterator it;
   for (it = book.begin(); it != book.end(); ++it) {
      *this += PhraseBookEntry ((*it).getPhrase(), (*it).getLevel()+1, (*it).isPhrase());
   }
}

// ***************************************************************************

PhraseBookDrag::PhraseBookDrag (PhraseBook *book, QWidget *dragSource, const char *name)
    : Q3DragObject (dragSource, name)
{
   setBook (book);
}

PhraseBookDrag::PhraseBookDrag (QWidget *dragSource, const char *name)
    : Q3DragObject (dragSource, name)
{
   setBook (0);
}

PhraseBookDrag::~PhraseBookDrag () {
}

void PhraseBookDrag::setBook (PhraseBook *book) {
   if (book == 0) {
      isEmpty = true;
      xmlphrasebook.setText(QString());
      xml.setText(QString());
      plain.setText(QString());
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
   QByteArray m(mime);
   m = m.toLower();
   if (m.contains("xml-phrasebook"))
      return xmlphrasebook.encodedData(mime);
   else if (m.contains("xml"))
      return xml.encodedData(mime);
   else
      return plain.encodedData(mime);
}

bool PhraseBookDrag::canDecode (const QMimeSource* e) {
   return Q3TextDrag::canDecode(e);
}

bool PhraseBookDrag::decode (const QMimeSource *e, PhraseBook *book) {
   QString string;
   QString subtype1 = "x-xml-phrasebook";
   QString subtype2 = "xml";

   if (!Q3TextDrag::decode(e, string, subtype1))
      if (!Q3TextDrag::decode(e, string, subtype2)) {
         if (Q3TextDrag::decode(e, string)) {
            *book += PhraseBookEntry(Phrase(string, ""), 0, true);
            return true;
         }
         else return false;
      }

   return book->decode(string);
}

#include "phrasebook.moc"
