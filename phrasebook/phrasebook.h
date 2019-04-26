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

#ifndef PHRASEBOOK_H
#define PHRASEBOOK_H

#include <QObject>
#include <QMenu>
#include <QPrinter>
#include <QTextStream>
#include <QXmlInputSource>

#include <QAction>
#include <KActionCollection>
#include <QIcon>
#include <KToolBar>

class QUrl;

struct StandardBook {
    QString name;
    QString path;
    QString filename;
};
typedef QList<StandardBook> StandardBookList;

/**
 * The class Phrase represents one phrase in a phrase book.
 * @author Gunnar Schmi Dt
 */
class Phrase
{
    friend class PhraseBookParser;
public:
    Phrase();
    Phrase(const QString &phrase);
    Phrase(const QString &phrase, const QString &shortcut);

    QString getPhrase() const;
    QString getShortcut() const;

    void setPhrase(const QString &phrase);
    void setShortcut(const QString &shortcut);

private:
    QString phrase;
    QString shortcut;
};

/**
 * The class PhraseBookEntry implements a phrase book entry. That can be either
 * a phrase or a start tag a sub phrase book.
 * @author Gunnar Schmi Dt
 */
class PhraseBookEntry
{
public:
    PhraseBookEntry();
    explicit PhraseBookEntry(const Phrase &phrase, int level = 1, bool isPhrase = true);
    ~PhraseBookEntry() {}

    void setPhrase(Phrase phrase, int level = 1, bool isPhrase = true);

    bool isPhrase() const;
    Phrase getPhrase() const;
    int getLevel() const;

private:
    bool isPhraseValue;
    Phrase phrase;
    int level;
};

typedef QList<PhraseBookEntry> PhraseBookEntryList;

/**
 * The class PhraseBook implements a phrase book. It mainly stores a
 * token list where each token is a phrase book entry (either a phrase
 * or a sub phrase book). The entries are placed into a tree structure
 * as follows:
 *
 * The level of each entry tells the level in the tree (level=0 is the top
 * level), each sub book in level i directly or indirectly contains all
 * following entries until an entry of level at most i or the end of the
 * token list.
 *
 * @author Gunnar Schmi Dt
 */
class PhraseBook : public PhraseBookEntryList
{
public:
    PhraseBook() : PhraseBookEntryList() {}
    ~PhraseBook() {}

    /** opens a file containing a phrase book. Returns true if successful. */
    bool open(const QUrl &url);

    /** decodes a phrase book. Returns true if successful. */
    bool decode(const QString &xml);

    /** decodes a phrase book. Returns true if successful. */
    bool decode(QXmlInputSource &source);

    /** Writes the phrases to a file. Returns true if successful. */
    bool save(const QUrl &url);

    /** Writes the phrases to a file. Returns true if successful. */
    bool save(const QUrl &url, bool asPhrasebook);

    /** Writes the phrases to a QTextStream. */
    void save(QTextStream &stream, bool asPhrasebook);

    /** Prints the phrases. */
    void print(QPrinter *pPrinter);

    /** Shows a file selector and writes the phrases to a file.
     *  @return 1, if the file got successfully written,
     *          0, if the user canceled the operation,
     *         -1, if there was an error when saving the file.
     */
    int save(QWidget *parent, const QString &title, QUrl &url, bool phrasebookFirst = true);

    /** encodes the phrase book. Returns the encoded xml code. */
    QString encode();

    /** Stores all entries in a QStringList. All hierarchy information and all
     * shortcuts are ignored during this operation.
     */
    QStringList toStringList();

    /** Adds the entries of the book to both the given popup menu and the given
     * toolbar. The corresponding actions will be inserted into phrases.
     */
    void addToGUI(QMenu *popup, KToolBar *toolbar,
                  KActionCollection *phrases,
                  QObject *receiver, const char *slot) const;

    /** Inserts book into a new sub phrase book.
     * @param name the name of the new sub phrase book.
     * @param book the phrase book to insert.
     */
    void insert(const QString &name, const PhraseBook &book);

    static StandardBookList standardPhraseBooks();

private:
    static QString displayPath(const QString &path);
};

class PhraseAction : public QAction
{
    Q_OBJECT
public:
    PhraseAction(const QString& phrase, const QString& cut, const QObject* receiver, const char* slot, KActionCollection* parent)
        : QAction(QIcon::fromTheme(QStringLiteral("phrase")), phrase, parent)
    {
        this->setShortcut(cut);
        this->phrase = phrase;
        connect(this, &QAction::triggered, this, &PhraseAction::slotTriggered);
        connect(this, SIGNAL(slotActivated(const QString &)), receiver, slot);
        parent->addAction(phrase, this);
    }
    ~PhraseAction()
    {
    }

public Q_SLOTS:
    void slotTriggered()
    {
//      trigger();
        emit slotActivated(phrase);
    }

Q_SIGNALS:
    void slotActivated(const QString &phrase);

private:
    QString phrase;
};

#endif
