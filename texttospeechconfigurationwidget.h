/***************************************************************************
                          texttospeechconfigurationdialog.h  -  description
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
    TextToSpeechConfigurationWidget(QWidget *parent, const char *name);
    ~TextToSpeechConfigurationWidget();

    TextToSpeechSystem *getTTSSystem() const;

    void readOptions(const QString &langGroup);
    void saveOptions(const QString &langGroup);

    void ok();
    void cancel();

private:
    void buildCodecList();

    TextToSpeechSystem *ttsSystem;
};

#endif // TEXTTOSPEECHCONFIGURATIONWIDGET_H
