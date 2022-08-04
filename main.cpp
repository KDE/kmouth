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

#include <QApplication>
#include <QCommandLineParser>
#include <QDir>

#include <KAboutData>
#include <KLocalizedString>

#include "kmouth.h"
#include "version.h"


int main(int argc, char *argv[])
{

    QApplication app(argc, argv);
    KLocalizedString::setApplicationDomain("kmouth");
    QApplication::setApplicationDisplayName(i18n("KMouth"));

    KAboutData aboutData(QStringLiteral("kmouth"),
                         i18n("KMouth"),
                         QStringLiteral(KMOUTH_VERSION_STRING),
                         i18n("A type-and-say front end for speech synthesizers"),
                         KAboutLicense::GPL,
                         i18n("(c) 2002/2003, Gunnar Schmi Dt"),
                         QString(),
                         QStringLiteral("https://www.kde.org/applications/utilities/kmouth/"),
                         QStringLiteral("kmouth@schmi-dt.de"));
    aboutData.addAuthor(i18n("Gunnar Schmi Dt"), i18n("Original Author"), QStringLiteral("kmouth@schmi-dt.de"));
    aboutData.addAuthor(i18n("Jeremy Whiting"), i18n("Current Maintainer"), QStringLiteral("jpwhiting@kde.org"));
    aboutData.addCredit(i18n("Olaf Schmidt"), i18n("Tips, extended phrase books"));

    aboutData.setOrganizationDomain(QByteArray("kde.org"));
    QCommandLineParser parser;
    KAboutData::setApplicationData(aboutData);
    parser.addPositionalArgument(QStringLiteral("[File]"), i18n("History file to open"));
    aboutData.setupCommandLine(&parser);
    parser.process(app);
    aboutData.processCommandLine(&parser);


    if (app.isSessionRestored()) {
        kRestoreMainWindows<KMouthApp>();
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
