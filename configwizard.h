/***************************************************************************
                          configwizard.h  -  description
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

// $Id$

#ifndef CONFIGWIZARD_H
#define CONFIGWIZARD_H

#include <qwidget.h>

#include <kwizard.h>
#include <kconfig.h>

#include "texttospeechconfigurationwidget.h"
#include "phrasebook/phrasebookdialog.h"

/**The class ConfigWizard is used when the user starts KMouth for the first
 * time. It asks the user to provide a first set of configuration data.
 *@author Gunnar Schmi Dt
 */

class ConfigWizard : public KWizard  {
   Q_OBJECT
public:
   ConfigWizard (QWidget *parent, const char *name, KConfig *config);
   ~ConfigWizard();

   bool configurationNeeded ();
   bool requestConfiguration ();
   void saveConfig (KConfig *config);

protected:
   void help();

private:
   void initCommandPage(KConfig *config);
   void initBookPage();

   TextToSpeechConfigurationWidget *commandWidget;
   InitialPhraseBookWidget *bookWidget;
};

#endif

/*
 * $Log$
 * Revision 1.5  2003/01/17 16:03:00  gunnar
 * Help buutons added and small bug when aborting the wizard fixed
 *
 * Revision 1.4  2002/12/30 12:08:07  gunnar
 * Configuration wizard improved
 *
 * Revision 1.3  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.2  2002/11/22 14:51:39  gunnar
 * Wizard for first start extended
 *
 * Revision 1.1  2002/11/21 21:33:26  gunnar
 * Extended parameter dialog and added wizard for the first start
 *
 */
