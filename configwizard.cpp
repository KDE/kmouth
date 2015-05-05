/***************************************************************************
                          configwizard.cpp  -  description
                             -------------------
    begin                : Mit Nov 20 2002
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

#include "configwizard.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>

#include <k3listview.h>
#include <klocale.h>
#include <kapplication.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <ktoolinvocation.h>

#include "texttospeechconfigurationwidget.h"
#include "phrasebook/initialphrasebookwidget.h"
#include "wordcompletion/wordcompletion.h"
#include "wordcompletion/dictionarycreationwizard.h"

ConfigWizard::ConfigWizard(QWidget *parent, KConfig *config)
    : QWizard(parent), m_config(config)
{
    setWindowTitle(i18n("Initial Configuration - KMouth"));
    initCommandPage();
    initBookPage();
    initCompletion();
    connect(this, SIGNAL(accepted()), this, SLOT(saveConfig()));
}

ConfigWizard::~ConfigWizard()
{
}

void ConfigWizard::initCommandPage()
{
    KConfigGroup cg(m_config, QLatin1String("TTS System"));
    bool displayCommand = false;
    if (!cg.hasKey("StdIn"))   displayCommand = true;
    if (!cg.hasKey("Codec"))   displayCommand = true;

    if (displayCommand) {
        commandWidget = new TextToSpeechConfigurationWidget(this, "ttsPage");
        commandWidget->readOptions(m_config, QLatin1String("TTS System"));
        commandWidget->setTitle(i18n("Text-to-Speech Configuration"));
        addPage(commandWidget);
        commandWidget->setFinalPage(true);
    } else
        commandWidget = 0;
}

void ConfigWizard::initBookPage()
{
    QString standardBook = KGlobal::dirs()->findResource("appdata", QLatin1String("standard.phrasebook"));
    bool displayBook = (standardBook.isNull() || standardBook.isEmpty());

    if (displayBook) {
        bookWidget = new InitialPhraseBookWidget(this, "pbPage");
        bookWidget->setTitle(i18n("Initial Phrase Book"));
        addPage(bookWidget);
        bookWidget->setFinalPage(true);
        if (commandWidget != 0)
            commandWidget->setFinalPage(false);
    } else
        bookWidget = 0;
}

void ConfigWizard::initCompletion()
{
    if (!WordCompletion::isConfigured()) {
        QString dictionaryFile = KGlobal::dirs()->findResource("appdata", QLatin1String("dictionary.txt"));
        QFile file(dictionaryFile);
        if (file.exists()) {
            // If there is a word completion dictionary but no entry in the
            // configuration file, we need to add it there.
            KConfigGroup cg(m_config , "Dictionary 0");
            cg.writeEntry("Filename", "dictionary.txt");
            cg.writeEntry("Name",     "Default");
            cg.writeEntry("Language", QString());
            cg.sync();
        }
    }

    if (m_config->hasGroup("Completion")) {
        completionWidget = 0;
        return;
    }

    if (!WordCompletion::isConfigured()) {
        completionWidget = new CompletionWizardWidget(this, "completionPage");
        completionWidget->setTitle(i18n("Word Completion"));
        addPage(completionWidget);
        completionWidget->setFinalPage(true);

        if (commandWidget != 0)
            commandWidget->setFinalPage(false);
        if (bookWidget != 0)
            bookWidget->setFinalPage(false);
    } else
        completionWidget = 0;
}

void ConfigWizard::saveConfig()
{
    if (commandWidget != 0) {
        commandWidget->ok();
        commandWidget->saveOptions(m_config, QLatin1String("TTS System"));
    }

    if (bookWidget != 0)
        bookWidget->createBook();

    if (completionWidget != 0)
        completionWidget->ok(m_config);
}

bool ConfigWizard::requestConfiguration()
{
    if (commandWidget != 0 || bookWidget != 0 || completionWidget != 0)
        return (exec() == QDialog::Accepted);
    else
        return false;
}

bool ConfigWizard::configurationNeeded()
{
    return (commandWidget != 0 || bookWidget != 0 || completionWidget != 0);
}

void ConfigWizard::help()
{
    KToolInvocation::invokeHelp(QLatin1String("Wizard"));
}

#include "configwizard.moc"
