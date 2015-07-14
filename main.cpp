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

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kuniqueapplication.h>

#include "kmouth.h"
#include "version.h"

static const char description[] =
    I18N_NOOP("A type-and-say front end for speech synthesizers");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


int main(int argc, char *argv[])
{

    KAboutData aboutData("kmouth", 0, ki18n("KMouth"),
                         KMOUTH_VERSION, ki18n(description), KAboutData::License_GPL,
                         ki18n("(c) 2002/2003, Gunnar Schmi Dt"), KLocalizedString(), "http://www.schmi-dt.de/kmouth/index.en.html", "kmouth@schmi-dt.de");
    aboutData.addAuthor(ki18n("Gunnar Schmi Dt"), ki18n("Original Author"), "kmouth@schmi-dt.de");
    aboutData.addAuthor(ki18n("Jeremy Whiting"), ki18n("Current Maintainer"), "jpwhiting@kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("+[File]", ki18n("History file to open"));
    KCmdLineArgs::addCmdLineOptions(options);   // Add our own options.

    aboutData.addCredit(ki18n("Olaf Schmidt"), ki18n("Tips, extended phrase books"));
    KApplication app;

    if (app.isSessionRestored()) {
        RESTORE(KMouthApp);
    } else {
        KMouthApp *kmouth = new KMouthApp();
        if (!kmouth->configured())
            return 0;

        kmouth->show();

        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        if (args->count()) {
            kmouth->openDocumentFile(args->url(0));
        }
        args->clear();
    }
    return app.exec();
}
