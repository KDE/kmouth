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

#include <QtTextToSpeech>

TextToSpeechConfigurationWidget::TextToSpeechConfigurationWidget(QWidget *parent, const QString &name)
    : QWizardPage(parent)
{
    setObjectName(name);
    setupUi(this);
    ttsSystem = new TextToSpeechSystem(this);

    connect(qtspeechGroupBox, &QGroupBox::toggled, this, &TextToSpeechConfigurationWidget::useQtspeechChanged);
    buildCodecList();

    // BEGIN Text-to-speech section
    // Populate tts engines and use their names directly as key and item text:
    const QStringList engines = QTextToSpeech::availableEngines();
    for (const QString &engine : engines) {
        engineComboBox->addItem(engine);
    }

    connect(engineComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &TextToSpeechConfigurationWidget::slotTTSEngineChanged);
    connect(voiceComboBox, &QComboBox::currentTextChanged, this, &TextToSpeechConfigurationWidget::slotTTSVoiceChanged);

    // Preload voice list
    slotTTSEngineChanged();
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
    qtspeechGroupBox->setChecked(ttsSystem->useQtSpeech);
}

void TextToSpeechConfigurationWidget::useQtspeechChanged(bool enabled)
{
    alternativeGroupBox->setEnabled(!enabled);
}

void TextToSpeechConfigurationWidget::slotTTSEngineChanged()
{
    QString engine = engineComboBox->currentText();
    ttsSystem->ttsEngine = engine;

    // Get list of voices and repopulate voice tts combobox
    QTextToSpeech *ttsEngine = new QTextToSpeech(engine);
    const QVector<QVoice> voices = ttsEngine->availableVoices();
    voiceComboBox->blockSignals(true);
    voiceComboBox->clear();
    for (const QVoice &voice : voices) {
        voiceComboBox->addItem(voice.name());
    }
    delete ttsEngine;

    // If there's a voice set, try to load it
    if (!ttsSystem->ttsVoice.isEmpty()) {
        voiceComboBox->setCurrentText(ttsSystem->ttsVoice);
    }
    voiceComboBox->blockSignals(false);
}

void TextToSpeechConfigurationWidget::slotTTSVoiceChanged(const QString &voice)
{
    ttsSystem->ttsVoice = voice;
}

void TextToSpeechConfigurationWidget::ok()
{
    ttsSystem->ttsCommand = urlReq->url().toLocalFile();
    ttsSystem->stdIn = stdInButton->isChecked();
    ttsSystem->codec = characterCodingBox->currentIndex();
    ttsSystem->useQtSpeech = qtspeechGroupBox->isChecked();
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
    qtspeechGroupBox->setChecked(ttsSystem->useQtSpeech);
    engineComboBox->setCurrentText(ttsSystem->ttsEngine);

    if (!ttsSystem->ttsVoice.isEmpty()) {
        voiceComboBox->setCurrentText(ttsSystem->ttsVoice);
    }
}

void TextToSpeechConfigurationWidget::saveOptions(const QString &langGroup)
{
    ttsSystem->saveOptions(langGroup);
}
