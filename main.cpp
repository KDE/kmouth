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
  KCmdLineLastOption
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
