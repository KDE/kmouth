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

#include "optionsdialog.h"

#include <QDialog>
#include <QDialogButtonBox>
#include <QIcon>
#include <QPushButton>
#include <QTabWidget>

#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>

#include "speech.h"
#include "texttospeechconfigurationwidget.h"
#include "wordcompletion/wordcompletionwidget.h"

PreferencesWidget::PreferencesWidget(QWidget *parent, const QString &name)
    : QWidget(parent)
{
    setObjectName(name);
    setupUi(this);
    speakCombo->setCurrentIndex(1);
    speak = false;

    closeCombo->setCurrentIndex(2);
    save = 2;
}

PreferencesWidget::~PreferencesWidget()
{
}

void PreferencesWidget::cancel()
{
    if (speak)
        speakCombo->setCurrentIndex(0);
    else
        speakCombo->setCurrentIndex(1);
    closeCombo->setCurrentIndex(save);
}

void PreferencesWidget::ok()
{
    speak = speakCombo->currentIndex() == 0;
    save = closeCombo->currentIndex();
}

void PreferencesWidget::readOptions()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "Preferences");
    if (cg.hasKey("AutomaticSpeak"))
        if (cg.readEntry("AutomaticSpeak") == QLatin1String("Yes"))
            speak = true;
        else
            speak = false;
    else
        speak = false;

    KConfigGroup cg2(KSharedConfig::openConfig(), "Notification Messages");
    if (cg2.hasKey("AutomaticSave"))
        if (cg2.readEntry("AutomaticSave") == QLatin1String("Yes"))
            save = 0;
        else
            save = 1;
    else
        save = 2;

    if (speak)
        speakCombo->setCurrentIndex(0);
    else
        speakCombo->setCurrentIndex(1);
    closeCombo->setCurrentIndex(save);
}

void PreferencesWidget::saveOptions()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "Preferences");
    if (speak)
        cg.writeEntry("AutomaticSpeak", "Yes");
    else
        cg.writeEntry("AutomaticSpeak", "No");

    KConfigGroup cg2(KSharedConfig::openConfig(), "Notification Messages");
    if (save == 0)
        cg2.writeEntry("AutomaticSave", "Yes");
    else if (save == 1)
        cg2.writeEntry("AutomaticSave", "No");
    else
        cg2.deleteEntry("AutomaticSave");
}

bool PreferencesWidget::isSpeakImmediately()
{
    return speak;
}

/***************************************************************************/

OptionsDialog::OptionsDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setWindowTitle(i18n("Configuration"));
    setFaceType(KPageDialog::List);
    // setHelp(QLatin1String("config-dialog"));
    setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help | QDialogButtonBox::Apply);

    // addGridPage (1, Qt::Horizontal, i18n("General Options"), QString(), iconGeneral);

    tabCtl = new QTabWidget();
    tabCtl->setObjectName(QStringLiteral("general"));

    behaviourWidget = new PreferencesWidget(tabCtl, QStringLiteral("prefPage"));
    tabCtl->addTab(behaviourWidget, i18n("&Preferences"));

    commandWidget = new TextToSpeechConfigurationWidget(tabCtl, QStringLiteral("ttsTab"));
    tabCtl->addTab(commandWidget, i18n("&Text-to-Speech"));

    KPageWidgetItem *pageGeneral = new KPageWidgetItem(tabCtl, i18n("General Options"));
    pageGeneral->setHeader(i18n("General Options"));
    pageGeneral->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    addPage(pageGeneral);

    completionWidget = new WordCompletionWidget(nullptr, "Word Completion widget");
    KPageWidgetItem *pageCompletion = new KPageWidgetItem(completionWidget, i18n("Word Completion"));
    pageCompletion->setHeader(i18n("Word Completion"));
    pageCompletion->setIcon(QIcon::fromTheme(QStringLiteral("keyboard")));
    addPage(pageCompletion);
    connect(button(QDialogButtonBox::Ok), &QAbstractButton::clicked, this, &OptionsDialog::slotOk);
    connect(button(QDialogButtonBox::Cancel), &QAbstractButton::clicked, this, &OptionsDialog::slotCancel);
    connect(button(QDialogButtonBox::Apply), &QAbstractButton::clicked, this, &OptionsDialog::slotApply);
}

OptionsDialog::~OptionsDialog()
{
}

void OptionsDialog::slotCancel()
{
    //   QDialog::slotCancel();
    commandWidget->cancel();
    behaviourWidget->cancel();
    completionWidget->load();
}

void OptionsDialog::slotOk()
{
    //   QDialog::slotOk();
    commandWidget->ok();
    behaviourWidget->ok();
    completionWidget->save();
    Q_EMIT configurationChanged();
}

void OptionsDialog::slotApply()
{
    //   QDialog::slotApply();
    commandWidget->ok();
    behaviourWidget->ok();
    completionWidget->save();
    Q_EMIT configurationChanged();
}

TextToSpeechSystem *OptionsDialog::getTTSSystem() const
{
    return commandWidget->getTTSSystem();
}

void OptionsDialog::readOptions()
{
    commandWidget->readOptions(QStringLiteral("TTS System"));
    behaviourWidget->readOptions();
}

void OptionsDialog::saveOptions()
{
    commandWidget->saveOptions(QStringLiteral("TTS System"));
    behaviourWidget->saveOptions();
    KSharedConfig::openConfig()->sync();
}

bool OptionsDialog::isSpeakImmediately()
{
    return behaviourWidget->isSpeakImmediately();
}
