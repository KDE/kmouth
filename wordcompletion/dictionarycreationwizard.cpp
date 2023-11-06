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

#include "dictionarycreationwizard.h"
#include "wordlist.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QLocale>
#include <QProgressDialog>
#include <QSpinBox>
#include <QStandardPaths>
#include <QStringConverter>

#include <QDebug>

#include <KConfigGroup>
#include <KSharedConfig>

int CreationSourceWidget::nextId() const
{
    int nextPage = -1;
    if (fileButton->isChecked()) {
        nextPage = DictionaryCreationWizard::FilePage;
    } else if (directoryButton->isChecked()) {
        nextPage = DictionaryCreationWizard::DirPage;
    } else if (kdeDocButton->isChecked()) {
        nextPage = DictionaryCreationWizard::KDEDocPage;
    } else if (mergeButton->isChecked()) {
        nextPage = DictionaryCreationWizard::MergePage;
    }
    return nextPage;
}

void CreationSourceWidget::emptyToggled(bool checked)
{
    setFinalPage(checked);
}

DictionaryCreationWizard::DictionaryCreationWizard(QWidget *parent,
                                                   const QStringList &dictionaryNames,
                                                   const QStringList &dictionaryFiles,
                                                   const QStringList &dictionaryLanguages)
    : QWizard(parent)
{
    creationSource = new CreationSourceWidget(this, QStringLiteral("source page"));
    creationSource->setTitle(i18n("Source of New Dictionary (1)"));
    setPage(CreationSourcePage, creationSource);
    setOption(QWizard::HaveHelpButton, false);
    // setFinishEnabled (creationSource, false);

    fileWidget = new CreationSourceDetailsWidget(this, QStringLiteral("file source page"));
    fileWidget->setTitle(i18n("Source of New Dictionary (2)"));
    fileWidget->setFinalPage(true);
    setPage(FilePage, fileWidget);
    buildCodecCombo(fileWidget->encodingCombo);

    dirWidget = new CreationSourceDetailsWidget(this, QStringLiteral("directory source page"));
    dirWidget->setTitle(i18n("Source of New Dictionary (2)"));
    dirWidget->setFinalPage(true);
    setPage(DirPage, dirWidget);
    dirWidget->urlLabel->setText(i18nc("In which directory is the file located?", "&Directory:"));
    dirWidget->urlLabel->setWhatsThis(i18n("With this input field you specify which directory you want to load for creating the new dictionary."));
    dirWidget->url->setMode(KFile::Directory);
    dirWidget->url->setWhatsThis(i18n("With this input field you specify which directory you want to load for creating the new dictionary."));
    buildCodecCombo(dirWidget->encodingCombo);

    kdeDocWidget = new KDEDocSourceWidget(this, "KDE documentation source page");
    kdeDocWidget->setTitle(i18n("Source of New Dictionary (2)"));
    kdeDocWidget->setFinalPage(true);
    setPage(KDEDocPage, kdeDocWidget);
    kdeDocWidget->languageButton->showLanguageCodes(true);
    kdeDocWidget->languageButton->loadAllLanguages();

    mergeWidget = new MergeWidget(this, dictionaryNames, dictionaryFiles, dictionaryLanguages);
    mergeWidget->setTitle(i18n("Source of New Dictionary (2)"));
    mergeWidget->setFinalPage(true);
    setPage(MergePage, mergeWidget);
}

DictionaryCreationWizard::~DictionaryCreationWizard()
{
}

void DictionaryCreationWizard::buildCodecCombo(KComboBox *combo)
{
    for (int encoding = QStringConverter::System; encoding <= QStringConverter::System; ++encoding) {
        QString name = QLatin1String(QStringConverter::nameForEncoding(QStringConverter::Encoding(encoding)));
        combo->addItem(name, encoding);
    }
}

QString DictionaryCreationWizard::createDictionary()
{
    WordList::WordMap map;
    QString dicFile;
    QProgressDialog *pdlg = WordList::progressDialog();

    if (creationSource->mergeButton->isChecked()) {
        map = WordList::mergeFiles(mergeWidget->mergeParameters(), pdlg);
        dicFile.clear();
    } else if (creationSource->emptyButton->isChecked()) {
        dicFile.clear();
    } else if (creationSource->fileButton->isChecked()) {
        QString filename = fileWidget->url->url().path();
        QStringConverter::Encoding encoding = QStringConverter::Encoding(fileWidget->encodingCombo->currentIndex());
        if (fileWidget->spellCheckBox->isChecked())
            dicFile = fileWidget->ooDictURL->url().path();
        map = WordList::parseFile(filename, encoding, pdlg);
    } else if (creationSource->directoryButton->isChecked()) {
        QString directory = dirWidget->url->url().path();
        QStringConverter::Encoding encoding = QStringConverter::Encoding(fileWidget->encodingCombo->currentIndex());
        if (dirWidget->spellCheckBox->isChecked())
            dicFile = dirWidget->ooDictURL->url().path();
        map = WordList::parseDir(directory, encoding, pdlg);
    } else { // creationSource->kdeDocButton must be checked
        QString language = kdeDocWidget->languageButton->current();
        if (kdeDocWidget->spellCheckBox->isChecked())
            dicFile = kdeDocWidget->ooDictURL->url().path();
        map = WordList::parseKDEDoc(language, pdlg);
    }

    if (!dicFile.isEmpty() && !dicFile.isNull())
        map = WordList::spellCheck(map, dicFile, pdlg);
    pdlg->close();
    delete pdlg;

    int dictnumber = 0;
    QString filename;
    QString dictionaryFile;
    do {
        dictnumber++;
        filename = QStringLiteral("wordcompletion%1.dict").arg(dictnumber);
        dictionaryFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, filename);
    } while (QFile::exists(dictionaryFile));

    dictionaryFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + QStringLiteral("/") + filename;
    if (WordList::saveWordList(map, dictionaryFile))
        return filename;
    else
        return QLatin1String("");
}

QString DictionaryCreationWizard::name()
{
    if (creationSource->mergeButton->isChecked()) {
        return i18n("Merge result");
    } else if (creationSource->emptyButton->isChecked()) {
        return i18nc("In the sense of a blank word list", "Empty list");
    } else if (creationSource->fileButton->isChecked()) {
        return fileWidget->url->url().path();
    } else if (creationSource->directoryButton->isChecked()) {
        return dirWidget->url->url().path();
    } else { // creationSource->kdeDocButton must be checked
        return i18n("KDE Documentation");
    }
}

QString DictionaryCreationWizard::language()
{
    if (creationSource->mergeButton->isChecked()) {
        return mergeWidget->language();
    } else if (creationSource->emptyButton->isChecked()) {
        return QLocale::languageToString(QLocale::system().language());
    } else if (creationSource->fileButton->isChecked()) {
        return fileWidget->languageButton->current();
    } else if (creationSource->directoryButton->isChecked()) {
        return dirWidget->languageButton->current();
    } else { // creationSource->kdeDocButton must be checked
        return kdeDocWidget->languageButton->current();
    }
}

/***************************************************************************/

MergeWidget::MergeWidget(QWidget *parent, const QStringList &dictionaryNames, const QStringList &dictionaryFiles, const QStringList &dictionaryLanguages)
    : QWizardPage(parent)
    , scrollArea(nullptr)
{
    QGridLayout *layout = new QGridLayout(this);
    layout->setColumnStretch(0, 0);
    layout->setColumnStretch(1, 1);

    int row = 0;
    QStringList::ConstIterator nIt = dictionaryNames.begin();
    QStringList::ConstIterator fIt = dictionaryFiles.begin();
    QStringList::ConstIterator lIt = dictionaryLanguages.begin();
    for (; nIt != dictionaryNames.end(); ++nIt, ++fIt, ++lIt) {
        QCheckBox *checkbox = new QCheckBox(*nIt, this);
        QSpinBox *spinBox = new QSpinBox(this);
        layout->addWidget(checkbox, row, 0);
        layout->addWidget(spinBox, row, 1);

        checkbox->setChecked(true);
        spinBox->setRange(1, 100);
        spinBox->setSingleStep(10);
        spinBox->setValue(100);
        connect(checkbox, &QAbstractButton::toggled, spinBox, &QWidget::setEnabled);

        dictionaries.insert(*fIt, checkbox);
        weights.insert(*fIt, spinBox);
        languages[*fIt] = *lIt;
        row++;
    }
    setLayout(layout);
}

MergeWidget::~MergeWidget()
{
}

QMap<QString, int> MergeWidget::mergeParameters()
{
    QMap<QString, int> files;
    QHashIterator<QString, QCheckBox *> it(dictionaries);
    while (it.hasNext()) {
        it.next();
        if (it.value()->isChecked()) {
            QString name = it.key();
            QString dictionaryFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, name);
            files[dictionaryFile] = weights.value(name)->value();
        }
    }

    return files;
}

QString MergeWidget::language()
{
    QHashIterator<QString, QCheckBox *> it(dictionaries);
    while (it.hasNext()) {
        it.next();
        if (it.value()->isChecked()) {
            return languages[it.key()];
        }
    }

    return QString();
}

/***************************************************************************/

CompletionWizardWidget::CompletionWizardWidget(QWidget *parent, const char *name)
    : QWizardPage(parent)
{
    setupUi(this);
    setObjectName(QLatin1String(name));
    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();
}

CompletionWizardWidget::~CompletionWizardWidget()
{
}

void CompletionWizardWidget::ok()
{
    WordList::WordMap map;
    QProgressDialog *pdlg = WordList::progressDialog();

    QString language = languageButton->current();
    map = WordList::parseKDEDoc(language, pdlg);

    if (spellCheckBox->isChecked())
        map = WordList::spellCheck(map, ooDictURL->url().path(), pdlg);

    pdlg->close();
    delete pdlg;

    QString dictionaryFile;

    dictionaryFile = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QLatin1Char('/') + QStringLiteral("/");
    qDebug() << "dictionaryFile is " << dictionaryFile;
    if (WordList::saveWordList(map, dictionaryFile)) {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("Dictionary 0"));
        cg.writeEntry("Filename", "wordcompletion1.dict");
        cg.writeEntry("Name", i18nc("Default dictionary", "Default"));
        cg.writeEntry("Language", language);
        cg.sync();
    }
}
