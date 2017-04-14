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

#include "texttospeechconfigurationwidget.h"

#include <QTextCodec>

#include <KLocalizedString>

#include "speech.h"
#include "texttospeechsystem.h"

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
    QString local = i18nc("Local characterset", "Local") + QStringLiteral(" (");
    local += QLatin1String(QTextCodec::codecForLocale()->name()) + QLatin1Char(')');
    characterCodingBox->addItem(local, Speech::Local);
    characterCodingBox->addItem(i18nc("Latin1 characterset", "Latin1"), Speech::Latin1);
    characterCodingBox->addItem(i18n("Unicode"), Speech::Unicode);
    for (int i = 0; i < ttsSystem->codecList->count(); i++)
        characterCodingBox->addItem(QLatin1String(ttsSystem->codecList->at(i)->name()), Speech::UseCodec + i);
}

void TextToSpeechConfigurationWidget::cancel()
{
    urlReq->setUrl(QUrl::fromLocalFile(ttsSystem->ttsCommand));
    stdInButton->setChecked(ttsSystem->stdIn);
    characterCodingBox->setCurrentIndex(ttsSystem->codec);
    useQtSpeech->setChecked(ttsSystem->useQtSpeech);
}

void TextToSpeechConfigurationWidget::ok()
{
    ttsSystem->ttsCommand = urlReq->url().toLocalFile();
    ttsSystem->stdIn = stdInButton->isChecked();
    ttsSystem->codec = characterCodingBox->currentIndex();
    ttsSystem->useQtSpeech = useQtSpeech->isChecked();
}

TextToSpeechSystem *TextToSpeechConfigurationWidget::getTTSSystem() const
{
    return ttsSystem;
}

void TextToSpeechConfigurationWidget::readOptions(const QString &langGroup)
{
    ttsSystem->readOptions(langGroup);
    urlReq->setUrl(QUrl::fromLocalFile(ttsSystem->ttsCommand));
    stdInButton->setChecked(ttsSystem->stdIn);
    characterCodingBox->setCurrentIndex(ttsSystem->codec);
    useQtSpeech->setChecked(ttsSystem->useQtSpeech);
}

void TextToSpeechConfigurationWidget::saveOptions(const QString &langGroup)
{
    ttsSystem->saveOptions(langGroup);
}

