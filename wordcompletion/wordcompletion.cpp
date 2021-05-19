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

#include "wordcompletion.h"

#include <QFile>
#include <QList>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QTextStream>

#include <KConfigGroup>
#include <KSharedConfig>

class WordCompletion::WordCompletionPrivate
{
    friend class WordCompletion;
private:
    typedef QMap<QString, int> WordMap;
    struct DictionaryDetails {
        QString filename;
        QString language;
    };

    QString lastText;
    QMap<QString, int> map;
    QMap<QString, int> addedWords;
    QMap<QString, DictionaryDetails> dictDetails;
    QStringList dictionaries;
    QString current;
    bool blockCurrentListSignal;
    bool wordsToSave;
};

WordCompletion::WordCompletion() : KCompletion()
{
    d = new WordCompletionPrivate();
    d->blockCurrentListSignal = false;
    d->wordsToSave = false;
    configure();
}

WordCompletion::~WordCompletion()
{
    save();
    delete d;
}

typedef QPair<int, QString> Match;
typedef QList<Match> MatchList;

QString WordCompletion::makeCompletion(const QString &text)
{
    if (d->lastText != text) {
        d->lastText = text;
        KCompletion::clear();

        int border = text.lastIndexOf(QRegularExpression(QStringLiteral("\\W")));
        QString suffix = text.right(text.length() - border - 1).toLower();
        QString prefix = text.left(border + 1);

        if (suffix.length() > 0) {
            MatchList matches;
            QMap<QString, int>::ConstIterator it;
            for (it = d->map.constBegin(); it != d->map.constEnd(); ++it)
                if (it.key().startsWith(suffix))
                    matches += Match(-it.value(), it.key());
            std::sort(matches.begin(), matches.end());

            MatchList::ConstIterator iter = matches.constBegin();
            for (int count = 0; (iter != matches.constEnd()) && (count < 10); ++iter, ++count) {
                int length = (*iter).second.length() + prefix.length() - text.length();
                KCompletion::addItem(text + (*iter).second.right(length), -(*iter).first);
            }
        }
    }

    // call the KCompletion::makeCompletion(...) method
    return KCompletion::makeCompletion(text);
}

QStringList WordCompletion::wordLists()
{
    return d->dictionaries;
}

QStringList WordCompletion::wordLists(const QString &language)
{
    QStringList result;
    for (QStringList::const_iterator it = d->dictionaries.constBegin();
         it != d->dictionaries.constEnd(); ++it)
        if (d->dictDetails[*it].language == language)
            result += *it;
    return result;
}

QString WordCompletion::languageOfWordList(const QString &wordlist)
{
    if (d->dictDetails.contains(wordlist))
        return d->dictDetails[wordlist].language;
    else
        return QString();
}

QString WordCompletion::currentWordList()
{
    return d->current;
}

bool WordCompletion::isConfigured()
{
    bool result = KSharedConfig::openConfig()->hasGroup("Dictionary 0");

    return result;
}

void WordCompletion::configure()
{
    if (d->wordsToSave)
        save();
    d->wordsToSave = false;

    d->dictionaries.clear();
    d->dictDetails.clear();

    const QStringList groups = KSharedConfig::openConfig()->groupList();
    for (QStringList::const_iterator it = groups.constBegin(); it != groups.constEnd(); ++it)
        if ((*it).startsWith(QLatin1String("Dictionary "))) {
            KConfigGroup cg(KSharedConfig::openConfig(), *it);
            WordCompletionPrivate::DictionaryDetails details;
            details.filename = cg.readEntry("Filename");
            details.language = cg.readEntry("Language");
            QString name = cg.readEntry("Name");
            d->dictDetails[name] = details;
            d->dictionaries += name;
        }

    d->blockCurrentListSignal = true;
    setWordList(d->current);
    d->blockCurrentListSignal = false;
    Q_EMIT wordListsChanged(wordLists());
    Q_EMIT currentListChanged(d->current);
}

bool WordCompletion::setWordList(const QString &wordlist)
{
    if (d->wordsToSave)
        save();
    d->wordsToSave = false;

    d->map.clear();
    bool result = d->dictDetails.contains(wordlist);
    if (result)
        d->current = wordlist;
    else {
        if (d->dictionaries.isEmpty()) return false;
        d->current = d->dictionaries[0];
    }

    QString filename = d->dictDetails[d->current].filename;
    QString dictionaryFile = QStandardPaths::locate(QStandardPaths::DataLocation, filename);
    QFile file(dictionaryFile);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        if (!stream.atEnd()) {
            if (stream.readLine() == QLatin1String("WPDictFile")) {
                while (!stream.atEnd()) {
                    QString s = stream.readLine();
                    if (!(s.isNull() || s.isEmpty())) {
                        QStringList list = s.split(QLatin1Char('\t'));
                        bool ok;
                        int weight = list[1].toInt(&ok);
                        if (ok && (weight > 0))
                            d->map [list[0]] = weight;
                    }
                }
            }
        }
        file.close();
    }
    if (!d->blockCurrentListSignal)
        Q_EMIT currentListChanged(d->current);
    d->lastText.clear();
    d->wordsToSave = false;
    return result;
}

void WordCompletion::addSentence(const QString &sentence)
{
    const QStringList words = sentence.split(QRegularExpression(QStringLiteral("\\W")));

    QStringList::ConstIterator it;
    for (it = words.constBegin(); it != words.constEnd(); ++it) {
        if (!(*it).contains(QRegularExpression(QStringLiteral("\\d|_")))) {
            QString key = (*it).toLower();
            if (d->map.contains(key))
                d->map[key] += 1;
            else
                d->map[key] = 1;
            if (d->addedWords.contains(key))
                d->addedWords[key] += 1;
            else
                d->addedWords[key] = 1;
        }
    }
    d->wordsToSave = true;
}

void WordCompletion::save()
{
    if (d->wordsToSave) {
        QString filename = d->dictDetails[d->current].filename;
        QString dictionaryFile = QStandardPaths::locate(QStandardPaths::DataLocation, filename);
        QFile file(dictionaryFile);
        if (!file.exists())
            return;
        if (!file.open(QIODevice::WriteOnly))
            return;

        QTextStream stream(&file);
        stream.setCodec("UTF-8");

        stream << "WPDictFile\n";
        QMap<QString, int>::ConstIterator it;
        for (it = d->map.constBegin(); it != d->map.constEnd(); ++it) {
            if (d->addedWords.contains(it.key())) {
                stream << it.key() << "\t" << d->addedWords[it.key()] << "\t1\n";
                stream << it.key() << "\t" << it.value() - d->addedWords[it.key()] << "\t2\n";
            } else
                stream << it.key() << "\t" << it.value() << "\t2\n";
        }
        file.close();
        d->wordsToSave = false;
    }
}

