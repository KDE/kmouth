/***************************************************************************
                          texttospeechsystem.h  -  description
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

// $Id$

#ifndef TEXTTOSPEECHSYSTEM_H
#define TEXTTOSPEECHSYSTEM_H

#include <qstring.h>
#include <qobject.h>
#include <qptrlist.h>
#include <kconfig.h>

/**This class represents a text-to-speech system.
  *@author Gunnar Schmi Dt
  */

class TextToSpeechSystem : public QObject{
   Q_OBJECT
   friend class TextToSpeechConfigurationWidget;
public: 
   TextToSpeechSystem();
   ~TextToSpeechSystem();

   void readOptions (KConfig *config, const QString &langGroup);
   void saveOptions (KConfig *config, const QString &langGroup);

public slots:
   void speak (QString text);

private:
   void buildCodecList ();

   QPtrList<QTextCodec> *codecList;
   int codec;
   QString ttsCommand;
   bool stdIn;
   bool useKttsd;
};

#endif

/*
 * $Log$
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.4  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.3  2002/11/21 21:33:27  gunnar
 * Extended parameter dialog and added wizard for the first start
 *
 * Revision 1.2  2002/11/04 16:38:42  gunnar
 * Incorporated changes for version 0.5.1 into head branch
 *
 * Revision 1.1.2.1  2002/11/04 15:36:37  gunnar
 * combo box for character encoding added
 *
 * Revision 1.1  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 */
