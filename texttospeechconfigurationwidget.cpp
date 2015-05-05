/***************************************************************************
                          texttospeechconfigurationdialog.cpp  -  description
                             -------------------
    begin                : Son Sep 8 2002
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

#include "texttospeechconfigurationwidget.h"
#include <kconfig.h>

#include <QTextCodec>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

#include <kcombobox.h>
#include <KLocalizedString>
#include "speech.h"
#include <kurlrequester.h>

TextToSpeechConfigurationWidget::TextToSpeechConfigurationWidget(QWidget *parent, const char *name)
    : QWizardPage(parent)
{
    setObjectName(QLatin1String(name));
    setupUi(this);
    ttsSystem = new TextToSpeechSystem();

    buildCodecList();
}

TextToSpeechConfigurationWidget::~TextToSpeechConfigurationWidget()
{
}

void TextToSpeechConfigurationWidget::buildCodecList()
{
    QString local = i18nc("Local characterset", "Local") + QLatin1String(" (");
    local += QLatin1String(QTextCodec::codecForLocale()->name()) + QLatin1Char(')');
    characterCodingBox->addItem(local, Speech::Local);
    characterCodingBox->addItem(i18nc("Latin1 characterset", "Latin1"), Speech::Latin1);
    characterCodingBox->addItem(i18n("Unicode"), Speech::Unicode);
    for (int i = 0; i < ttsSystem->codecList->count(); i++)
        characterCodingBox->addItem(QLatin1String(ttsSystem->codecList->at(i)->name()), Speech::UseCodec + i);
}

void TextToSpeechConfigurationWidget::cancel()
{
    urlReq->setUrl(ttsSystem->ttsCommand);
    stdInButton->setChecked(ttsSystem->stdIn);
    characterCodingBox->setCurrentIndex(ttsSystem->codec);
    useQtSpeech->setChecked(ttsSystem->useQtSpeech);
}

void TextToSpeechConfigurationWidget::ok()
{
    ttsSystem->ttsCommand = urlReq->url().path();
    ttsSystem->stdIn = stdInButton->isChecked();
    ttsSystem->codec = characterCodingBox->currentIndex();
    ttsSystem->useQtSpeech = useQtSpeech->isChecked();
}

TextToSpeechSystem *TextToSpeechConfigurationWidget::getTTSSystem() const
{
    return ttsSystem;
}

void TextToSpeechConfigurationWidget::readOptions(KConfig *config, const QString &langGroup)
{
    ttsSystem->readOptions(config, langGroup);
    urlReq->setUrl(ttsSystem->ttsCommand);
    stdInButton->setChecked(ttsSystem->stdIn);
    characterCodingBox->setCurrentIndex(ttsSystem->codec);
    useQtSpeech->setChecked(ttsSystem->useQtSpeech);
}

void TextToSpeechConfigurationWidget::saveOptions(KConfig *config, const QString &langGroup)
{
    ttsSystem->saveOptions(config, langGroup);
}

