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

#include "configwizard.h"

#include <QStandardPaths>

#include <KConfigGroup>
#include <KHelpClient>
#include <KLocalizedString>
#include <KSharedConfig>

#include "phrasebook/initialphrasebookwidget.h"
#include "texttospeechconfigurationwidget.h"
#include "wordcompletion/dictionarycreationwizard.h"
#include "wordcompletion/wordcompletion.h"

ConfigWizard::ConfigWizard(QWidget *parent)
    : QWizard(parent)
{
    setWindowTitle(i18n("Initial Configuration - KMouth"));
    initCommandPage();
    initBookPage();
    initCompletion();
    connect(this, &QDialog::accepted, this, &ConfigWizard::saveConfig);
}

ConfigWizard::~ConfigWizard()
{
}

void ConfigWizard::initCommandPage()
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("TTS System"));
    bool displayCommand = false;
    if (!cg.hasKey("StdIn"))
        displayCommand = true;
    if (!cg.hasKey("Codec"))
        displayCommand = true;

    if (displayCommand) {
        commandWidget = new TextToSpeechConfigurationWidget(this, QStringLiteral("ttsPage"));
        commandWidget->readOptions(QStringLiteral("TTS System"));
        commandWidget->setTitle(i18n("Text-to-Speech Configuration"));
        addPage(commandWidget);
        commandWidget->setFinalPage(true);
    } else
        commandWidget = nullptr;
}

void ConfigWizard::initBookPage()
{
    QString standardBook = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("standard.phrasebook"));
    bool displayBook = (standardBook.isNull() || standardBook.isEmpty());

    if (displayBook) {
        bookWidget = new InitialPhraseBookWidget(this, "pbPage");
        bookWidget->setTitle(i18n("Initial Phrase Book"));
        addPage(bookWidget);
        bookWidget->setFinalPage(true);
        if (commandWidget != nullptr)
            commandWidget->setFinalPage(false);
    } else
        bookWidget = nullptr;
}

void ConfigWizard::initCompletion()
{
    if (!WordCompletion::isConfigured()) {
        QString dictionaryFile = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("dictionary.txt"));
        QFile file(dictionaryFile);
        if (file.exists()) {
            // If there is a word completion dictionary but no entry in the
            // configuration file, we need to add it there.
            KConfigGroup cg(KSharedConfig::openConfig(), "Dictionary 0");
            cg.writeEntry("Filename", "dictionary.txt");
            cg.writeEntry("Name", "Default");
            cg.writeEntry("Language", QString());
            cg.sync();
        }
    }

    if (KSharedConfig::openConfig()->hasGroup("Completion")) {
        completionWidget = nullptr;
        return;
    }

    if (!WordCompletion::isConfigured()) {
        completionWidget = new CompletionWizardWidget(this, "completionPage");
        completionWidget->setTitle(i18n("Word Completion"));
        addPage(completionWidget);
        completionWidget->setFinalPage(true);

        if (commandWidget != nullptr)
            commandWidget->setFinalPage(false);
        if (bookWidget != nullptr)
            bookWidget->setFinalPage(false);
    } else
        completionWidget = nullptr;
}

void ConfigWizard::saveConfig()
{
    if (commandWidget != nullptr) {
        commandWidget->ok();
        commandWidget->saveOptions(QStringLiteral("TTS System"));
    }

    if (bookWidget != nullptr)
        bookWidget->createBook();

    if (completionWidget != nullptr)
        completionWidget->ok();
}

bool ConfigWizard::requestConfiguration()
{
    if (commandWidget != nullptr || bookWidget != nullptr || completionWidget != nullptr)
        return (exec() == QDialog::Accepted);
    else
        return false;
}

bool ConfigWizard::configurationNeeded()
{
    return (commandWidget != nullptr || bookWidget != nullptr || completionWidget != nullptr);
}

void ConfigWizard::help()
{
    KHelpClient::invokeHelp(QStringLiteral("Wizard"));
}
