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

#ifndef CONFIGWIZARD_H
#define CONFIGWIZARD_H

#include <QWizard>

class TextToSpeechConfigurationWidget;
class InitialPhraseBookWidget;
class CompletionWizardWidget;

/**The class ConfigWizard is used when the user starts KMouth for the first
 * time. It asks the user to provide a first set of configuration data.
 *@author Gunnar Schmi Dt
 */
class ConfigWizard : public QWizard
{
    Q_OBJECT
public:
    explicit ConfigWizard(QWidget *parent);
    ~ConfigWizard() override;

    bool configurationNeeded();
    bool requestConfiguration();

public Q_SLOTS:
    void saveConfig();

protected:
    void help();

private:
    void initCommandPage();
    void initBookPage();
    void initCompletion();

    TextToSpeechConfigurationWidget *commandWidget;
    InitialPhraseBookWidget *bookWidget;
    CompletionWizardWidget *completionWidget;
};

#endif
