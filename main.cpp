/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mon Aug 26 15:41:23 CEST 2002
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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kuniqueapplication.h>

#include "kmouth.h"
#include "version.h"

static const char *description =
	I18N_NOOP("A type-and-say front end for speech synthesizers");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE
	
	
static KCmdLineOptions options[] =
{
  { "+[File]", I18N_NOOP("History file to open"), 0 },
  { 0, 0, 0 }
  // INSERT YOUR COMMANDLINE OPTIONS HERE
};

int main(int argc, char *argv[])
{

	KAboutData aboutData( "kmouth", I18N_NOOP("KMouth"),
		KMOUTH_VERSION, description, KAboutData::License_GPL,
		"(c) 2002/2003, Gunnar Schmi Dt", 0, "http://www.schmi-dt.de/kmouth/index.en.html", "kmouth@schmi-dt.de");
	aboutData.addAuthor("Gunnar Schmi Dt",0, "kmouth@schmi-dt.de");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options ); // Add our own options.

	aboutData.addCredit("Olaf Schmidt", I18N_NOOP("Tips, extended phrase books"), 0, 0);
  KApplication app;
 
  if (app.isRestored())
  {
    RESTORE(KMouthApp);
  }
  else 
  {
    KMouthApp *kmouth = new KMouthApp();
    if (!kmouth->configured())
       return 0;

    kmouth->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
		
		if (args->count())
		{
        kmouth->openDocumentFile(args->arg(0));
		}
		args->clear();
  }
  return app.exec();
}  

/*
 * $Log$
 * Revision 1.3  2003/01/19 21:53:08  gunnar
 * corrected version number of KMouth
 *
 * Revision 1.2  2003/01/18 07:29:11  binner
 * CVS_SILENT i18n style guide fixes
 *
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.6  2002/12/30 12:08:07  gunnar
 * Configuration wizard improved
 *
 * Revision 1.5  2002/11/21 15:40:28  gunnar
 * changed KMouth back to be a standard KApplication
 *
 * Revision 1.4  2002/11/19 17:48:14  gunnar
 * Prevented both the parallel start of multiple KMouth instances and the parallel opening of multiple Phrase book edit windows
 *
 * Revision 1.3  2002/10/02 16:55:17  gunnar
 * Removed some compiler warnings and implementing the feature of opening a history at the start of KMouth
 *
 * Revision 1.2  2002/10/02 14:55:33  gunnar
 * Fixed Speak-empty-phrase-crash bug and added some i18n() encodings
 *
 * Revision 1.1.1.1  2002/08/26 14:09:49  gunnar
 * New project started
 *
 */
