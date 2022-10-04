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

#include "phrasebook.h"
#include "phrasebookreader.h"

#include <QBuffer>
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QPainter>
#include <QStack>
#include <QUrl>

#include <KActionMenu>
#include <KDesktopFile>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>

#include <QtDebug>

Phrase::Phrase()
{
    this->phrase.clear();
    this->shortcut.clear();
}

Phrase::Phrase(const QString &phrase)
{
    this->phrase = phrase;
    this->shortcut.clear();
}

Phrase::Phrase(const QString &phrase, const QString &shortcut)
{
    this->phrase = phrase;
    this->shortcut = shortcut;
}

QString Phrase::getPhrase() const
{
    return phrase;
}

QString Phrase::getShortcut() const
{
    return shortcut;
}

void Phrase::setPhrase(const QString &phrase)
{
    this->phrase = phrase;
}

void Phrase::setShortcut(const QString &shortcut)
{
    this->shortcut = shortcut;
}

// ***************************************************************************

PhraseBookEntry::PhraseBookEntry()
{
    level = 1;
    isPhraseValue = false;
}

PhraseBookEntry::PhraseBookEntry(const Phrase &phrase, int level, bool isPhrase)
{
    this->phrase = phrase;
    this->level = level;
    isPhraseValue = isPhrase;
}

bool PhraseBookEntry::isPhrase() const
{
    return isPhraseValue;
}

Phrase PhraseBookEntry::getPhrase() const
{
    return phrase;
}

int PhraseBookEntry::getLevel() const
{
    return level;
}

// ***************************************************************************

void PhraseBook::print(QPrinter *pPrinter)
{
    QPainter printpainter;
    printpainter.begin(pPrinter);

    QRect size = printpainter.viewport();
    int x = size.x();
    int y = size.y();
    int w = size.width();
    printpainter.setFont(QFont(QFontDatabase::systemFont(QFontDatabase::GeneralFont).family(), 12));
    QFontMetrics metrics = printpainter.fontMetrics();

    PhraseBookEntryList::iterator it;
    for (it = begin(); it != end(); ++it) {
        QRect rect = metrics.boundingRect(x + 16 * (*it).getLevel(), y,
                                          w - 16 * (*it).getLevel(), 0,
                                          Qt::AlignJustify | Qt::TextWordWrap,
                                          (*it).getPhrase().getPhrase());

        if (y + rect.height() > size.height()) {
            pPrinter->newPage();
            y = 0;
        }
        printpainter.drawText(x + 16 * (*it).getLevel(), y,
                              w - 16 * (*it).getLevel(), rect.height(),
                              Qt::AlignJustify | Qt::TextWordWrap,
                              (*it).getPhrase().getPhrase());
        y += rect.height();
    }

    printpainter.end();
}

bool PhraseBook::decode(QIODevice *source)
{
    PhraseBookReader reader;

    if (reader.read(source)) {
        PhraseBookEntryList::clear();
        *(PhraseBookEntryList *)this += reader.getPhraseList();
        return true;
    } else {
        qDebug() << "Unable to read xml file: " << reader.errorString();
        return false;
    }
}

QByteArray encodeString(const QString &str)
{
    QByteArray res = "";
    for (int i = 0; i < (int)str.length(); i++) {
        QChar ch = str.at(i);
        ushort uc = ch.unicode();
        QByteArray number; number.setNum(uc);
        if ((uc > 127) || (uc < 32) || (ch == QLatin1Char('<')) || (ch == QLatin1Char('>')) || (ch == QLatin1Char('&')) || (ch == QLatin1Char(';')))
            res = res + "&#" + number + ';';
        else
            res = res + (char)uc;
    }
    return res;
}

QString PhraseBook::encode()
{
    QString result;
    result  = QStringLiteral("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
    result += QLatin1String("<!DOCTYPE phrasebook>\n");
    result += QLatin1String("<phrasebook>\n");

    PhraseBookEntryList::iterator it;
    int level = 0;
    for (it = begin(); it != end(); ++it) {
        int newLevel = (*it).getLevel();
        while (level < newLevel) {
            result += QLatin1String("<phrasebook>\n");
            level++;
        }
        while (level > newLevel) {
            result += QLatin1String("</phrasebook>\n");
            level--;
        }

        if ((*it).isPhrase()) {
            Phrase phrase = (*it).getPhrase();
            result += QStringLiteral("<phrase shortcut=\"") + QLatin1String(encodeString(phrase.getShortcut()));
            result += QStringLiteral("\">") + QLatin1String(encodeString(phrase.getPhrase())) + QStringLiteral("</phrase>\n");
        } else {
            Phrase phrase = (*it).getPhrase();
            result += QStringLiteral("<phrasebook name=\"") + QLatin1String(encodeString(phrase.getPhrase())) + QStringLiteral("\">\n");
            level++;
        }
    }
    while (level > 0) {
        result += QLatin1String("</phrasebook>\n");
        level--;
    }
    result += QLatin1String("</phrasebook>");
    return result;
}

QStringList PhraseBook::toStringList()
{
    QStringList result;

    PhraseBook::iterator it;
    for (it = begin(); it != end(); ++it) {
        if ((*it).isPhrase())
            result += (*it).getPhrase().getPhrase();
    }
    return result;
}

bool PhraseBook::save(const QUrl &url)
{
    return save(url, url.fileName().endsWith(QLatin1String(".phrasebook")));
}


void PhraseBook::save(QTextStream &stream, bool asPhrasebook)
{
    if (asPhrasebook)
        stream << encode();
    else
        stream << toStringList().join(QLatin1String("\n"));
}

bool PhraseBook::save(const QUrl &url, bool asPhrasebook)
{
    if (url.isLocalFile()) {
        QFile file(url.path());
        if (!file.open(QIODevice::WriteOnly))
            return false;

        QTextStream stream(&file);
        save(stream, asPhrasebook);
        file.close();

        if (file.error() != QFile::NoError)
            return false;
        else
            return true;
    } else {
        QByteArray data;
        QTextStream ts(&data);
        save(ts, asPhrasebook);
        ts.flush();

        KIO::StoredTransferJob *uploadJob = KIO::storedPut(data, url, -1);
        return uploadJob->exec();
    }
}

int PhraseBook::save(QWidget *parent, const QString &title, QUrl &url, bool phrasebookFirst)
{
    // KFileDialog::getSaveUrl(...) is not useful here as we need
    // to know the requested file type.

    QString filters;
    if (phrasebookFirst)
        filters = i18n("Phrase Books (*.phrasebook);;Plain Text Files (*.txt);;All Files (*)");
    else
        filters = i18n("Plain Text Files (*.txt);;Phrase Books (*.phrasebook);;All Files (*)");

    QFileDialog fdlg(parent, title, QString(), filters);
    fdlg.setAcceptMode(QFileDialog::AcceptSave);

    if (fdlg.exec() != QDialog::Accepted || fdlg.selectedUrls().size() < 1) {
        return 0;
    }

    url = fdlg.selectedUrls().at(0);

    if (url.isEmpty() || !url.isValid()) {
        return -1;
    }

    if (QFile::exists(url.toLocalFile())) {
        if (KMessageBox::warningContinueCancel(nullptr, QStringLiteral("<qt>%1</qt>").arg(i18n("The file %1 already exists. "
                                               "Do you want to overwrite it?", url.url())), i18n("File Exists"), KGuiItem(i18n("&Overwrite"))) == KMessageBox::Cancel) {
            return 0;
        }
    }

    bool result;
    if (fdlg.selectedNameFilter() == QLatin1String("*.phrasebook")) {
        if (url.fileName(QUrl::PrettyDecoded).contains(QLatin1Char('.')) == 0) {
            url = url.adjusted(QUrl::RemoveFilename);
            url.setPath(url.path() + url.fileName(QUrl::PrettyDecoded) + QStringLiteral(".phrasebook"));
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        } else if (url.fileName(QUrl::PrettyDecoded).rightRef(11).contains(QLatin1String(".phrasebook"), Qt::CaseInsensitive) == 0) {
#else
        } else if (url.fileName(QUrl::PrettyDecoded).right(11).contains(QLatin1String(".phrasebook"), Qt::CaseInsensitive) == 0) {
#endif
            int filetype = KMessageBox::questionYesNoCancel(nullptr, QStringLiteral("<qt>%1</qt>").arg(i18n("Your chosen filename <i>%1</i> has a different extension than <i>.phrasebook</i>. "
                           "Do you wish to add <i>.phrasebook</i> to the filename?", url.fileName())), i18n("File Extension"), KGuiItem(i18n("Add")), KGuiItem(i18n("Do Not Add")));
            if (filetype == KMessageBox::Cancel) {
                return 0;
            }
            if (filetype == KMessageBox::Yes) {
                url = url.adjusted(QUrl::RemoveFilename);
                url.setPath(url.path() + url.fileName(QUrl::PrettyDecoded) + QStringLiteral(".phrasebook"));
            }
        }
        result = save(url, true);
    } else if (fdlg.selectedNameFilter() == QLatin1String("*.txt")) {
        if (url.fileName(QUrl::PrettyDecoded).rightRef(11).contains(QLatin1String(".phrasebook"), Qt::CaseInsensitive) == 0) {
            result = save(url, false);
        } else {
            int filetype = KMessageBox::questionYesNoCancel(nullptr, QStringLiteral("<qt>%1</qt>").arg(i18n("Your chosen filename <i>%1</i> has the extension <i>.phrasebook</i>. "
                           "Do you wish to save in phrasebook format?", url.fileName())), i18n("File Extension"), KGuiItem(i18n("As Phrasebook")), KGuiItem(i18n("As Plain Text")));
            if (filetype == KMessageBox::Cancel) {
                return 0;
            }
            if (filetype == KMessageBox::Yes) {
                result = save(url, true);
            } else {
                result = save(url, false);
            }
        }
    } else // file format "All files" requested, so decide by extension
        result = save(url);

    if (result)
        return 1;
    else
        return -1;
}

bool PhraseBook::open(const QUrl &url)
{
    KIO::StoredTransferJob *downloadJob = KIO::storedGet(url);
    if (downloadJob->exec()) {
        // First: try to load it as a normal phrase book
        QBuffer fileBuffer;
        fileBuffer.setData(downloadJob->data());

        fileBuffer.open(QIODevice::ReadOnly);

        bool error = !decode(&fileBuffer);

        // Second: if the file does not contain a phrase book, load it as
        // a plain text file
        if (error) {
            // Load each line of the plain text file as a new phrase

            QTextStream stream(&fileBuffer);

            while (!stream.atEnd()) {
                QString s = stream.readLine();
                if (!(s.isNull() || s.isEmpty()))
                    *this += PhraseBookEntry(Phrase(s, QLatin1String("")), 0, true);
            }
            error = false;
        }

        return !error;
    }
    return false;
}

StandardBookList PhraseBook::standardPhraseBooks()
{
    // Get all the standard phrasebook filenames in bookPaths.
    QStringList bookPaths;
    const QStringList dirs =
        QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("books"), QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QStringList locales = QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &locale: locales) {
            const QStringList fileNames =
                QDir(dir + QLatin1Char('/') + locale).entryList(QStringList() << QStringLiteral("*.phrasebook"));
            for (const QString &file : fileNames) {
                bookPaths.append(dir + QLatin1Char('/') + locale + QLatin1Char('/') + file);
            }
        }
    }
    QStringList bookNames;
    QMap<QString, StandardBook> bookMap;
    QStringList::iterator it;
    // Iterate over all books creating a phrasebook for each, creating a StandardBook of each.
    for (it = bookPaths.begin(); it != bookPaths.end(); ++it) {
        PhraseBook pbook;
        // Open the phrasebook.
        if (pbook.open(QUrl::fromLocalFile(*it))) {
            StandardBook book;
            book.name = (*pbook.begin()).getPhrase().getPhrase();

            book.path = displayPath(*it);
            book.filename = *it;

            bookNames += book.path + QLatin1Char('/') + book.name;
            bookMap [book.path + QLatin1Char('/') + book.name] = book;
        }
    }

    bookNames.sort();

    StandardBookList result;
    for (it = bookNames.begin(); it != bookNames.end(); ++it)
        result += bookMap [*it];

    return result;
}

QString PhraseBook::displayPath(const QString &filename)
{
    QFileInfo file(filename);
    QString path = file.path();
    QString dispPath;
    int position = path.indexOf(QLatin1String("/kmouth/books/")) + QStringLiteral("/kmouth/books/").length();

    while (path.length() > position) {
        file.setFile(path);

        KDesktopFile *dirDesc = new KDesktopFile(QStandardPaths::GenericDataLocation, path + QStringLiteral("/.directory"));
        QString name = dirDesc->readName();
        delete dirDesc;

        if (name.isNull() || name.isEmpty())
            dispPath += QLatin1Char('/') + file.fileName();
        else
            dispPath += QLatin1Char('/') + name;

        path = file.path();
    }
    return dispPath;
}

void PhraseBook::addToGUI(QMenu *popup, KToolBar *toolbar, KActionCollection *phrases,
                          QObject *receiver, const char *slot) const
{
    if ((popup != nullptr) || (toolbar != nullptr)) {
        QStack<QWidget*> stack;
        QWidget *parent = popup;
        int level = 0;

        QList<PhraseBookEntry>::ConstIterator it;
        for (it = begin(); it != end(); ++it) {
            int newLevel = (*it).getLevel();
            while (newLevel > level) {
                KActionMenu *menu = phrases->add<KActionMenu>(QStringLiteral("phrasebook"));
                menu->setPopupMode(QToolButton::InstantPopup);
                if (parent == popup)
                    toolbar->addAction(menu);
                if (parent != nullptr) {
                    parent->addAction(menu);
                    stack.push(parent);
                }
                parent = menu->menu();
                level++;
            }
            while (newLevel < level && (parent != popup)) {
                parent = stack.pop();
                level--;
            }
            if ((*it).isPhrase()) {
                Phrase phrase = (*it).getPhrase();
                QAction *action = new PhraseAction(phrase.getPhrase(),
                                                   phrase.getShortcut(), receiver, slot, phrases);
                if (parent == popup)
                    toolbar->addAction(action);
                if (parent != nullptr)
                    parent->addAction(action);
            } else {
                Phrase phrase = (*it).getPhrase();
                KActionMenu *menu = phrases->add<KActionMenu>(QStringLiteral("phrasebook"));
                menu->setText(phrase.getPhrase());
                menu->setPopupMode(QToolButton::InstantPopup);
                if (parent == popup)
                    toolbar->addAction(menu);
                parent->addAction(menu);
                stack.push(parent);
                parent = menu->menu();
                level++;
            }
        }
    }
}

void PhraseBook::insert(const QString &name, const PhraseBook &book)
{
    *this += PhraseBookEntry(Phrase(name), 0, false);

    QList<PhraseBookEntry>::ConstIterator it;
    for (it = book.begin(); it != book.end(); ++it) {
        *this += PhraseBookEntry((*it).getPhrase(), (*it).getLevel() + 1, (*it).isPhrase());
    }
}

