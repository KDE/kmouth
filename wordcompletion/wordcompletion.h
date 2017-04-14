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

#ifndef WORDCOMPLETION_H
#define WORDCOMPLETION_H

#include <kcompletion.h>

/**
 * This class does completion based on a dictionary of words.
 */
class WordCompletion : public KCompletion
{
    friend class WordListWidget;
    Q_OBJECT
public:
    WordCompletion();
    virtual ~WordCompletion();

    /**
     * Returns the names for the available word lists
     */
    QStringList wordLists();

    /**
     * Returns the names for those word lists that contain
     * words of a given language.
     */
    QStringList wordLists(const QString &language);

    /**
     * Returns the language of a given word list.
     */
    QString languageOfWordList(const QString &wordlist);

    /**
     * Returns the name of the currently active word list.
     */
    QString currentWordList();

    /**
     * Finds completions to the given text.
     */
    virtual QString makeCompletion(const QString&);

    static bool isConfigured();

    /**
     * Adds the words from the given sentence to the list of words.
     */
    void addSentence(const QString &sentence);

public slots:
    /**
     * Re-reads the configuration.
     */
    void configure();

    /**
     * Specify which word list gets used for the actual word completion.
     * If there is no word list with the given name the first configured
     * list gets used.
     * The method returns true if the specified word list was found.
     */
    bool setWordList(const QString &wordlist);

    /**
     * Saves the added words to disk.
     */
    void save();

signals:
    void wordListsChanged(const QStringList &wordLists);
    void currentListChanged(const QString &wordList);

private:
    class WordCompletionPrivate;
    WordCompletionPrivate *d;
};

#endif // KURLCOMPLETION_H
