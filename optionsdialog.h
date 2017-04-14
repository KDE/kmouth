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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <KPageDialog>

#include "ui_preferencesui.h"

class QTabWidget;
class KCModule;
class TextToSpeechSystem;
class TextToSpeechConfigurationWidget;
class WordCompletionWidget;

/**This class represents a configuration widget for user preferences.
  *@author Gunnar Schmi Dt
  */

class PreferencesWidget : public QWidget, public Ui::PreferencesUI
{
    Q_OBJECT
public:
    PreferencesWidget(QWidget *parent, const char *name);
    ~PreferencesWidget();

    void readOptions();
    void saveOptions();

    void ok();
    void cancel();

    bool isSpeakImmediately();

private:
    bool speak;
    int save;
};

/**This class represents a configuration dialog for the options of KMouth.
  *@author Gunnar Schmi Dt
  */

class OptionsDialog : public KPageDialog
{
    Q_OBJECT
public:
    OptionsDialog(QWidget *parent);
    ~OptionsDialog();

    TextToSpeechSystem *getTTSSystem() const;

    void readOptions();
    void saveOptions();

    bool isSpeakImmediately();

signals:
    void configurationChanged();

private slots:
    void slotCancel();
    void slotOk();
    void slotApply();

private:
    QTabWidget *tabCtl;
    TextToSpeechConfigurationWidget *commandWidget;
    PreferencesWidget *behaviourWidget;
    WordCompletionWidget *completionWidget;
};

#endif
