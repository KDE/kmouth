/***************************************************************************
                          wordcompletionwidget.cpp  -  description
                             -------------------
    begin                : Tue Apr 29 2003
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

#include "dictionarycreationwizard.h"
#include "wordlist.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QCheckBox>
#include <QtGui/QRadioButton>
#include <QtGui/QLineEdit>
#include <QtGui/QGridLayout>
#include <QtCore/QTextCodec>
#include <QtCore/QTextStream>
#include <QProgressDialog>
#include <QDebug>

#include <k3listview.h>
#include <klineedit.h>
#include <kurlrequester.h>
#include <klocale.h>
#include <kcombobox.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <klanguagebutton.h>

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

DictionaryCreationWizard::DictionaryCreationWizard(QWidget *parent, const char *name,
        const QStringList &dictionaryNames, const QStringList &dictionaryFiles,
        const QStringList &dictionaryLanguages)
    : QWizard(parent)
{
    buildCodecList();

    creationSource = new CreationSourceWidget(this, "source page");
    creationSource->setTitle(i18n("Source of New Dictionary (1)"));
    setPage(CreationSourcePage, creationSource);
    setOption(QWizard::HaveHelpButton, false);
    //setFinishEnabled (creationSource, false);

    fileWidget = new CreationSourceDetailsWidget(this, "file source page");
    fileWidget->setTitle(i18n("Source of New Dictionary (2)"));
    fileWidget->setFinalPage(true);
    setPage(FilePage, fileWidget);
    buildCodecCombo(fileWidget->encodingCombo);

    dirWidget = new CreationSourceDetailsWidget(this, "directory source page");
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

    mergeWidget = new MergeWidget(this, "merge source page", dictionaryNames, dictionaryFiles, dictionaryLanguages);
    mergeWidget->setTitle(i18n("Source of New Dictionary (2)"));
    mergeWidget->setFinalPage(true);
    setPage(MergePage, mergeWidget);
}

DictionaryCreationWizard::~DictionaryCreationWizard()
{
    delete codecList;
}

void DictionaryCreationWizard::buildCodecList()
{
    codecList = new QList<QTextCodec*>;
    QList<QByteArray> availableCodecs = QTextCodec::availableCodecs();
    for (int i = 0; i < availableCodecs.count(); ++i) {
        QTextCodec *codec = QTextCodec::codecForName(availableCodecs[i]);
        codecList->append(codec);
    }
}

void DictionaryCreationWizard::buildCodecCombo(KComboBox *combo)
{
    QString local = i18nc("Local characterset", "Local") + QLatin1String(" (");
    local += QLatin1String(QTextCodec::codecForLocale()->name()) + QLatin1Char(')');
    combo->addItem(local, 0);
    combo->addItem(i18nc("Latin characterset", "Latin1"), 1);
    combo->addItem(i18n("Unicode"), 2);

    for (int i = 0; i < codecList->count(); i++)
        combo->addItem(QLatin1String(codecList->at(i)->name()), i + 3);
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
        int encoding = fileWidget->encodingCombo->currentIndex();
        if (fileWidget->spellCheckBox->isChecked())
            dicFile = fileWidget->ooDictURL->url().path();
        switch (encoding) {
        case 0:
            map = WordList::parseFile(filename, QTextCodec::codecForLocale(), pdlg);
            break;
        case 1:
            map = WordList::parseFile(filename, QTextCodec::codecForName("ISO-8859-1"), pdlg);
            break;
        case 2:
            map = WordList::parseFile(filename, QTextCodec::codecForName("UTF-16"), pdlg);
            break;
        default:
            map = WordList::parseFile(filename, codecList->at(encoding - 3), pdlg);
        }
    } else if (creationSource->directoryButton->isChecked()) {
        QString directory = dirWidget->url->url().path();
        int encoding = dirWidget->encodingCombo->currentIndex();
        if (dirWidget->spellCheckBox->isChecked())
            dicFile = dirWidget->ooDictURL->url().path();
        switch (encoding) {
        case 0:
            map = WordList::parseDir(directory, QTextCodec::codecForLocale(), pdlg);
            break;
        case 1:
            map = WordList::parseDir(directory, QTextCodec::codecForName("ISO-8859-1"), pdlg);
            break;
        case 2:
            map = WordList::parseDir(directory, QTextCodec::codecForName("UTF-16"), pdlg);
            break;
        default:
            map = WordList::parseDir(directory, codecList->at(encoding - 3), pdlg);
        }
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
        filename = QString(QLatin1String("wordcompletion%1.dict")).arg(dictnumber);
        dictionaryFile = KGlobal::dirs()->findResource("appdata", filename);
    } while (KStandardDirs::exists(dictionaryFile));

    dictionaryFile = KGlobal::dirs()->saveLocation("appdata", QLatin1String("/")) + filename;
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
        if (KGlobal::locale())
            return KGlobal::locale()->language();
        else
            return KLocale::defaultLanguage();
    } else if (creationSource->fileButton->isChecked()) {
        return fileWidget->languageButton->current();
    } else if (creationSource->directoryButton->isChecked()) {
        return dirWidget->languageButton->current();
    } else { // creationSource->kdeDocButton must be checked
        return kdeDocWidget->languageButton->current();
    }
}

/***************************************************************************/

MergeWidget::MergeWidget(QWidget *parent, const char *name,
                         const QStringList &dictionaryNames, const QStringList &dictionaryFiles,
                         const QStringList &dictionaryLanguages)
    : QWizardPage(parent)
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
        KIntNumInput *numInput = new KIntNumInput(this);
        layout->addWidget(checkbox, row, 0);
        layout->addWidget(numInput, row, 1);

        checkbox->setChecked(true);
        numInput->setRange(1, 100, 10);
        numInput->setSliderEnabled(true);
        numInput->setValue(100);
        connect(checkbox, SIGNAL(toggled(bool)), numInput, SLOT(setEnabled(bool)));

        dictionaries.insert(*fIt, checkbox);
        weights.insert(*fIt, numInput);
        languages [*fIt] = *lIt;
        row++;
    }
    setLayout(layout);
}

MergeWidget::~MergeWidget()
{
}

QMap <QString, int> MergeWidget::mergeParameters()
{
    QMap <QString, int> files;
    QHashIterator<QString, QCheckBox*> it(dictionaries);
    while (it.hasNext()) {
        it.next();
        if (it.value()->isChecked()) {
            QString name = it.key();
            QString dictionaryFile = KGlobal::dirs()->findResource("appdata", name);
            files[dictionaryFile] = weights.value(name)->value();
        }
    }

    return files;
}

QString MergeWidget::language()
{
    QHashIterator<QString, QCheckBox*> it(dictionaries);
    while (it.hasNext()) {
        it.next();
        if (it.value()->isChecked()) {
            return languages [it.key()];
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
}

CompletionWizardWidget::~CompletionWizardWidget()
{
}

void CompletionWizardWidget::ok(KConfig *config)
{
    WordList::WordMap map;
    QProgressDialog *pdlg = WordList::progressDialog();

    QString language = languageButton->current();
    map = WordList::parseKDEDoc(language, pdlg);

    if (spellCheckBox->isChecked())
        map = WordList::spellCheck(map, ooDictURL->url().path(), pdlg);

    pdlg->close();
    delete pdlg;

    QString filename;
    QString dictionaryFile;

    dictionaryFile = KGlobal::dirs()->saveLocation("appdata", QLatin1String("/")) + QLatin1String("wordcompletion1.dict");
    qDebug() << "dictionaryFile is " << dictionaryFile;
    if (WordList::saveWordList(map, dictionaryFile)) {
        KConfigGroup cg(config, "Dictionary 0");
        cg.writeEntry("Filename", "wordcompletion1.dict");
        cg.writeEntry("Name",     i18nc("Default dictionary", "Default"));
        cg.writeEntry("Language", language);
        cg.sync();
    }
}

#include "dictionarycreationwizard.moc"
