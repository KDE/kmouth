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


#include <QApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QDir>

#include <KAboutData>
#include <KLocalizedString>

#include "kmouth.h"
#include "version.h"

static const char description[] =
    I18N_NOOP("A type-and-say front end for speech synthesizers");
// INSERT A DESCRIPTION FOR YOUR APPLICATION HERE


int main(int argc, char *argv[])
{

    QApplication::setApplicationName(QStringLiteral("kmouth"));
    QApplication::setApplicationVersion(KMOUTH_VERSION);
    QApplication::setOrganizationDomain(QStringLiteral("kde.org"));
    KLocalizedString::setApplicationDomain("kmouth");
    QApplication::setApplicationDisplayName(i18n("KMouth"));

    KAboutData aboutData(I18N_NOOP("kmouth"),
                         i18n("KMouth"),
                         KMOUTH_VERSION,
                         i18n(description),
                         KAboutLicense::GPL,
                         i18n("(c) 2002/2003, Gunnar Schmi Dt"),
                         QString(),
                         I18N_NOOP("http://www.schmi-dt.de/kmouth/index.en.html"),
                         I18N_NOOP("kmouth@schmi-dt.de"));
    aboutData.addAuthor(i18n("Gunnar Schmi Dt"), QString(), QStringLiteral("kmouth@schmi-dt.de"));
    QApplication app(argc, argv);
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addVersionOption();
    parser.addHelpOption();
    //PORTING SCRIPT: adapt aboutdata variable if necessary
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);

    parser.addPositionalArgument(QStringLiteral("[File]"), i18n("History file to open"));

    aboutData.addCredit(i18n("Olaf Schmidt"), i18n("Tips, extended phrase books"));

    if (app.isSessionRestored()) {
        RESTORE(KMouthApp);
    } else {
        KMouthApp *kmouth = new KMouthApp();
        if (!kmouth->configured())
            return 0;

        kmouth->show();


        if (!parser.positionalArguments().isEmpty()) {
            const QUrl url = QUrl::fromUserInput(parser.positionalArguments().at(0), QDir::currentPath());
            kmouth->openDocumentFile(url);
        }

    }
    return app.exec();
}
