/*
 * klanguagebuttonhelper.cpp - Methods that help filling the KLanguageButton with data.
 *
 * Copyright (c) 2003 Gunnar Schmi Dt <gunnar@schmi-dt.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.trolltech.com/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "klanguagebuttonhelper.h"

#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include "klanguagebutton.h"

QString languageName (QString languageCode) {
   QString filename = KGlobal::dirs()->findResource("locale",
			languageCode + QString::fromLatin1("/entry.desktop"));
      
   KConfig entry(filename, KConfig::OnlyLocal);
   entry.setGroup(QString::fromLatin1("KCM Locale"));
   return entry.readEntry(QString::fromLatin1("Name"), i18n("without name"));
}

void loadLanguageList(KLanguageButton *combo)
// This function was taken from kdebase/kcontrol/kdm/kdm-appear.cpp
{
  QStringList langlist = KGlobal::dirs()->findAllResources("locale",
			QString::fromLatin1("*/entry.desktop"));
  langlist.sort();
  for ( QStringList::ConstIterator it = langlist.begin();
	it != langlist.end(); ++it )
  {
    QString fpath = (*it).left((*it).length() - 14);
    int index = fpath.lastIndexOf('/');
    QString nid = fpath.mid(index + 1);

    KConfig entry(*it, KConfig::OnlyLocal);
    entry.setGroup(QString::fromLatin1("KCM Locale"));
    QString name = entry.readEntry(QString::fromLatin1("Name"), i18n("without name"));
    combo->insertLanguage(nid, name, QString::fromLatin1("l10n/"), QString());
  }
  
  if (KGlobal::locale())
     combo->setCurrentItem (KGlobal::locale()->language());
  else
     combo->setCurrentItem (KLocale::defaultLanguage());
}
