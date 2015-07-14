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
#include <kconfig.h>

#include <QtCore/QTextCodec>
#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QCheckBox>

#include <kcombobox.h>
#include <klocale.h>
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
    useKttsd->setChecked(ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::ok()
{
    ttsSystem->ttsCommand = urlReq->url().path();
    ttsSystem->stdIn = stdInButton->isChecked();
    ttsSystem->codec = characterCodingBox->currentIndex();
    ttsSystem->useKttsd = useKttsd->isChecked();
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
    useKttsd->setChecked(ttsSystem->useKttsd);
}

void TextToSpeechConfigurationWidget::saveOptions(KConfig *config, const QString &langGroup)
{
    ttsSystem->saveOptions(config, langGroup);
}

