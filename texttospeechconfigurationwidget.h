/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015, 2022 by Jeremy Whiting <jpwhiting@kde.org>        *
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

#ifndef TEXTTOSPEECHCONFIGURATIONWIDGET_H
#define TEXTTOSPEECHCONFIGURATIONWIDGET_H

#include "ui_texttospeechconfigurationui.h"

#include <QWizardPage>

class TextToSpeechSystem;

/**This class represents a configuration widget for the text-to-speech system.
 *@author Gunnar Schmi Dt
 */

class TextToSpeechConfigurationWidget : public QWizardPage, public Ui::texttospeechconfigurationui
{
    friend class TextToSpeechConfigurationDialog;

public:
    explicit TextToSpeechConfigurationWidget(QWidget *parent, const QString &name);
    ~TextToSpeechConfigurationWidget() override;

    TextToSpeechSystem *getTTSSystem() const;

    void readOptions(const QString &langGroup);
    void saveOptions(const QString &langGroup);

    void ok();
    void cancel();

private Q_SLOTS:
    void useQtspeechChanged(bool enabled);

    void slotTTSEngineChanged();

    void slotTTSVoiceChanged(const QString &voice);

private:
    void buildCodecList();

    TextToSpeechSystem *ttsSystem = nullptr;
};

#endif // TEXTTOSPEECHCONFIGURATIONWIDGET_H
