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

#include "wordlist.h"

#include <QApplication>
#include <QDir>
#include <QList>
#include <QProgressDialog>
#include <QRegExp>
#include <QStandardPaths>
#include <QTextCodec>
#include <QTextStream>
#include <QXmlAttributes>
#include <QXmlInputSource>
#include <QXmlParseException>
#include <QXmlSimpleReader>

#include <KLocalizedString>

namespace WordList
{
void addWords(WordMap &map, const QString &line);

XMLParser::XMLParser()
{
}

XMLParser::~XMLParser()
{
}

bool XMLParser::warning(const QXmlParseException &)
{
    return false;
}

bool XMLParser::error(const QXmlParseException &)
{
    return false;
}

bool XMLParser::fatalError(const QXmlParseException &)
{
    return false;
}

QString XMLParser::errorString() const
{
    return QLatin1String("");
}

bool XMLParser::startDocument()
{
    text.clear();
    return true;
}

bool XMLParser::startElement(const QString &, const QString &,
                             const QString &,
                             const QXmlAttributes &)
{
    if (!text.isNull() && !text.isEmpty()) {
        addWords(list, text);
        text.clear();
    }
    return true;
}

bool XMLParser::characters(const QString &ch)
{
    text += ch;
    return true;
}

bool XMLParser::ignorableWhitespace(const QString &)
{
    if (!text.isNull() && !text.isEmpty()) {
        addWords(list, text);
        text.clear();
    }
    return true;
}

bool XMLParser::endElement(const QString &, const QString &,
                           const QString &)
{
    if (!text.isNull() && !text.isEmpty()) {
        addWords(list, text);
        text.clear();
    }
    return true;
}

bool XMLParser::endDocument()
{
    if (!text.isNull() && !text.isEmpty()) {
        addWords(list, text);
        text.clear();
    }
    return true;
}

WordMap XMLParser::getList()
{
    return list;
}

/***************************************************************************/

QProgressDialog *progressDialog()
{
    QProgressDialog *pdlg = new QProgressDialog(i18n("Creating Word List"), i18n("Parsing the KDE documentation..."), 0, 100);
    pdlg->setCancelButton(nullptr);
    pdlg->setAutoReset(false);
    pdlg->setAutoClose(false);
    return pdlg;
}

bool saveWordList(const WordMap &map, const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly))
        return false;

    QTextStream stream(&file);
    stream.setCodec("UTF-8");

    stream << "WPDictFile\n";
    WordMap::ConstIterator it;
    for (it = map.constBegin(); it != map.constEnd(); ++it)
        stream << it.key() << "\t" << it.value() << "\t2\n";
    file.close();
    return true;
}

/***************************************************************************/

void addWords(WordMap &map, const QString &line)
{
    const QStringList words = line.split(QRegExp(QStringLiteral("\\W")));

    QStringList::ConstIterator it;
    for (it = words.constBegin(); it != words.constEnd(); ++it) {
        if (!(*it).contains(QRegExp(QStringLiteral("\\d|_")))) {
            QString key = (*it).toLower();
            if (map.contains(key))
                map[key] += 1;
            else
                map[key] = 1;
        }
    }
}

void addWords(WordMap &map, const WordMap &add)
{
    WordList::WordMap::ConstIterator it;
    for (it = add.constBegin(); it != add.constEnd(); ++it)
        if (map.contains(it.key()))
            map[it.key()] += it.value();
        else
            map[it.key()] = it.value();
}

void addWordsFromFile(WordMap &map, const QString &filename, QTextCodec *codec)
{
    QFile xmlfile(filename);
    QXmlInputSource source(&xmlfile);
    XMLParser parser;
    QXmlSimpleReader reader;
    reader.setFeature(QStringLiteral("http://qt-project.org/xml/features/report-start-end-entity"), true);
    reader.setContentHandler(&parser);

    WordMap words;
    if (reader.parse(source)) // try to load the file as an xml-file
        addWords(map, parser.getList());
    else {
        QFile wpdfile(filename);
        if (wpdfile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&wpdfile);
            stream.setCodec("UTF-8");

            if (!stream.atEnd()) {
                QString s = stream.readLine();
                if (s == QLatin1String("WPDictFile")) {   // Contains the file a weighted word list?
                    // We can assume that weighted word lists are always UTF8 coded.
                    while (!stream.atEnd()) {
                        QString s = stream.readLine();
                        if (!(s.isNull() || s.isEmpty())) {
                            QStringList list = s.split(QLatin1Char('\t'));
                            bool ok;
                            int weight = list[1].toInt(&ok);
                            if (ok && (weight > 0)) {
                                if (map.contains(list[0]))
                                    map[list[0]] += weight;
                                else
                                    map[list[0]] = weight;
                            }
                        }
                    }
                } else { // Count the words in an ordinary text file
                    QFile file(filename);
                    if (file.open(QIODevice::ReadOnly)) {
                        QTextStream stream(&file);
                        Q_ASSERT(codec != nullptr);
                        stream.setCodec(codec);
                        while (!stream.atEnd())
                            addWords(map, stream.readLine());
                    }
                }
            }
        }
    }
}

WordMap parseFiles(const QStringList &files, QTextCodec *codec, QProgressDialog *pdlg)
{
    int progress = 0;
    int steps = files.count();
    int percent = 0;

    WordMap map;
    QStringList::ConstIterator it;
    for (progress = 1, it = files.constBegin(); it != files.constEnd(); ++progress, ++it) {
        addWordsFromFile(map, *it, codec);

        if (steps != 0 && progress * 100 / steps > percent) {
            percent = progress * 100 / steps;
            pdlg->setValue(percent);
            qApp->processEvents(QEventLoop::AllEvents, 20);
        }
    }
    return map;
}

WordMap mergeFiles(const QMap<QString, int> &files, QProgressDialog *pdlg)
{
    pdlg->setLabelText(i18n("Merging dictionaries..."));
    pdlg->show();
    qApp->processEvents(QEventLoop::AllEvents, 20);

    int progress = 0;
    int steps = files.count();
    int percent = 0;
    float totalWeight = 0;
    long long maxWeight = 0;

    QMap<QString, float> map;
    QMap<QString, int>::ConstIterator it;
    for (progress = 1, it = files.constBegin(); it != files.constEnd(); ++progress, ++it) {
        WordMap fileMap;
        addWordsFromFile(fileMap, it.key(), QTextCodec::codecForName("UTF-8"));

        long long weight = 0;
        WordMap::ConstIterator iter;
        for (iter = fileMap.constBegin(); iter != fileMap.constEnd(); ++iter)
            weight += iter.value();
        float factor = 1.0 * it.value() / weight;
        totalWeight += it.value();
        if (weight > maxWeight)
            maxWeight = weight;

        for (iter = fileMap.constBegin(); iter != fileMap.constEnd(); ++iter)
            if (map.contains(iter.key()))
                map[iter.key()] += iter.value() * factor;
            else
                map[iter.key()] = iter.value() * factor;

        if (steps != 0 && progress * 100 / steps > percent) {
            percent = progress * 100 / steps;
            pdlg->setValue(percent);
            qApp->processEvents(QEventLoop::AllEvents, 20);
        }
    }

    float factor;
    if (1.0 * maxWeight * totalWeight > 1000000000)
        factor = 1000000000 / totalWeight;
    else
        factor = 1.0 * maxWeight;

    WordMap resultMap;
    QMap<QString, float>::ConstIterator iter;
    for (iter = map.constBegin(); iter != map.constEnd(); ++iter)
        resultMap[iter.key()] = (int)(factor * iter.value() + 0.5);

    return resultMap;
}

WordMap parseKDEDoc(QString language, QProgressDialog *pdlg)
{
    pdlg->setLabelText(i18n("Parsing the KDE documentation..."));
    pdlg->show();
    qApp->processEvents(QEventLoop::AllEvents, 20);

    QStringList files = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("html/") + language + QStringLiteral("/*.docbook"));
    if ((files.count() == 0) && (language.length() == 5)) {
        language = language.left(2);
        files = QStandardPaths::locateAll(QStandardPaths::GenericDataLocation, QStringLiteral("html/") + language + QStringLiteral("/*.docbook"));
    }

    return parseFiles(files, QTextCodec::codecForName("UTF-8"), pdlg);
}

WordMap parseFile(const QString &filename, QTextCodec *codec, QProgressDialog *pdlg)
{
    pdlg->setLabelText(i18n("Parsing file..."));
    pdlg->show();
    qApp->processEvents(QEventLoop::AllEvents, 20);

    QStringList files;
    files.append(filename);

    return parseFiles(files, codec, pdlg);
}

WordMap parseDir(const QString &directory, QTextCodec *codec, QProgressDialog *pdlg)
{
    pdlg->setLabelText(i18n("Parsing directory..."));
    pdlg->show();
    qApp->processEvents(QEventLoop::AllEvents, 20);

    QStringList directories;
    directories += directory;
    QStringList files;
    int dirNdx = 0;
    while (dirNdx < directories.count()) {
        QDir dir(directories.at(dirNdx));
        const QFileInfoList entries = dir.entryInfoList(QDir::Dirs | QDir::Files | QDir::NoSymLinks | QDir::Readable);

        for (int i = 0; i < entries.size(); ++i) {
            QFileInfo fileInfo = entries.at(i);

            QString name = fileInfo.fileName();
            if (name != QLatin1String(".") && name != QLatin1String("..")) {
                if (fileInfo.isDir())
                    directories += fileInfo.filePath();
                else
                    files += fileInfo.filePath();
            }
        }
        directories.removeAt(dirNdx);
    }

    return parseFiles(files, codec, pdlg);
}

/***************************************************************************/

/* Structures used for storing *.aff files (part of OpenOffice.org dictionaries)
 */
struct AffEntry {
    bool    cross;
    int     charsToRemove;
    QString add;
    QStringList condition;
};

typedef QList<AffEntry> AffList;
typedef QMap<QChar, AffList>  AffMap;

/** Loads an *.aff file (part of OpenOffice.org dictionaries)
 */
void loadAffFile(const QString &filename, AffMap &prefixes, AffMap &suffixes)
{
    bool cross = false;

    QFile afile(filename);
    if (afile.open(QIODevice::ReadOnly)) {
        QTextStream stream(&afile);
        while (!stream.atEnd()) {
            QString s = stream.readLine();
            QStringList fields = s.split(QRegExp(QStringLiteral("\\s")));

            if (fields.count() == 4) {
                cross = (fields[2] == QLatin1String("Y"));
            } else {
                if (fields.count() >= 5) {
                    AffEntry e;
                    e.cross     = cross;
                    if (fields[2] == QLatin1String("0"))
                        e.charsToRemove = 0;
                    else
                        e.charsToRemove = fields[2].length();
                    e.add       = fields[3];

                    if (fields[4] != QLatin1String(".")) {
                        QString condition = fields[4];
                        for (int idx = 0; idx < condition.length(); ++idx) {
                            if (condition[idx] == QLatin1Char('[')) {
                                QString code;
                                for (++idx; (idx < condition.length()) && condition[idx] != QLatin1Char(']'); ++idx)
                                    code += condition[idx];
                                e.condition << code;
                            } else
                                e.condition << QString(condition[idx]);
                        }
                    }

                    if (s.startsWith(QLatin1String("PFX"))) {
                        AffList list;
                        if (prefixes.contains(fields[1][0]))
                            list = prefixes[fields[1][0]];
                        list << e;
                        prefixes[fields[1][0]] = list;
                    } else if (s.startsWith(QLatin1String("SFX"))) {
                        AffList list;
                        if (suffixes.contains(fields[1][0]))
                            list = suffixes[fields[1][0]];
                        list << e;
                        suffixes[fields[1][0]] = list;
                    }
                }
            }
        }
    }
}

/** Checks if the given word matches the given condition. Each entry of the
 * QStringList "condition" describes one character of the word. (If the word
 * has more characters than condition entries only the last characters are
 * compared).
 * Each entry contains either all valid characters (if it does _not_ start
 * with "^") or all invalid characters (if it starts with "^").
 */
inline bool checkCondition(const QString &word, const QStringList &condition)
{
    if (condition.count() == 0)
        return true;

    if (word.length() < condition.count())
        return false;

    QStringList::ConstIterator it;
    int idx;
    for (it = condition.constBegin(), idx = word.length() - condition.count();
         it != condition.constEnd();
         ++it, ++idx) {
        if ((*it).contains(word[idx]) == ((*it)[0] == QLatin1Char('^')))
            return false;
    }
    return true;
}

/** Constructs words by adding suffixes to the given word, and copies the
 * resulting words from map to checkedMap.
 * @param modifiers discribes which suffixes are valid
 * @param cross true if the word has a prefix
 */
inline void checkWord(const QString &word, const QString &modifiers, bool cross, const WordMap &map, WordMap &checkedMap, const AffMap &suffixes)
{
    for (int i = 0; i < modifiers.length(); i++) {
        if (suffixes.contains(modifiers[i])) {
            AffList sList = suffixes[modifiers[i]];

            AffList::ConstIterator sIt;
            for (sIt = sList.constBegin(); sIt != sList.constEnd(); ++sIt) {
                if (((*sIt).cross || !cross)
                    && (checkCondition(word, (*sIt).condition))) {
                    QString sWord = word.left(word.length() - (*sIt).charsToRemove) + (*sIt).add;
                    if (map.contains(sWord))
                        checkedMap[sWord] = map[sWord];
                }
            }
        }
    }
}

/** Constructs words by adding pre- and suffixes to the given word, and
 * copies the resulting words from map to checkedMap.
 * @param modifiers discribes which pre- and suffixes are valid
 */
void checkWord(const QString &word, const QString &modifiers, const WordMap &map, WordMap &checkedMap, const AffMap &prefixes, const AffMap &suffixes)
{
    if (map.contains(word))
        checkedMap[word] = map[word];

    checkWord(word, modifiers, true, map, checkedMap, suffixes);

    for (int i = 0; i < modifiers.length(); i++) {
        if (prefixes.contains(modifiers[i])) {
            AffList pList = prefixes[modifiers[i]];

            AffList::ConstIterator pIt;
            for (pIt = pList.constBegin(); pIt != pList.constEnd(); ++pIt) {
                QString pWord = (*pIt).add + word;
                if (map.contains(pWord))
                    checkedMap[pWord] = map[pWord];

                checkWord(pWord, modifiers, false, map, checkedMap, suffixes);
            }
        }
    }
}

WordMap spellCheck(WordMap map, const QString &dictionary, QProgressDialog *pdlg)
{

    if (dictionary.endsWith(QLatin1String(".dic"))) {
        AffMap prefixes;
        AffMap suffixes;
        WordMap checkedMap;
        loadAffFile(dictionary.left(dictionary.length() - 4) + QStringLiteral(".aff"), prefixes, suffixes);

        pdlg->reset();
        //pdlg->setAllowCancel (false);
        //pdlg->showCancelButton (false);
        pdlg->setAutoReset(false);
        pdlg->setAutoClose(false);
        pdlg->setLabelText(i18n("Performing spell check..."));
        pdlg->setMaximum(100);
        pdlg->setValue(0);
        qApp->processEvents(QEventLoop::AllEvents, 20);
        int progress = 0;
        int steps = 0;
        int percent = 0;

        QFile dfile(dictionary);
        if (dfile.open(QIODevice::ReadOnly)) {
            QTextStream stream(&dfile);

            if (!stream.atEnd()) {
                QString s = stream.readLine(); // Number of words
                steps = s.toInt();
            }

            while (!stream.atEnd()) {
                QString s = stream.readLine();
                if (s.contains(QLatin1Char('/'))) {
                    QString word = s.left(s.indexOf(QLatin1Char('/'))).toLower();
                    QString modifiers = s.right(s.length() - s.indexOf(QLatin1Char('/')));

                    checkWord(word, modifiers, map, checkedMap, prefixes, suffixes);
                } else {
                    if (!s.isEmpty() && !s.isNull() && map.contains(s.toLower()))
                        checkedMap[s.toLower()] = map[s.toLower()];
                }

                progress++;
                if (steps != 0 && progress * 100 / steps > percent) {
                    percent = progress * 100 / steps;
                    pdlg->setValue(percent);
                    qApp->processEvents(QEventLoop::AllEvents, 20);
                }
            }
        }

        return checkedMap;
    } else
        return map;
}

}

