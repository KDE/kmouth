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


#ifndef TEXTTOSPEECHSYSTEM_H
#define TEXTTOSPEECHSYSTEM_H

#include <tqstring.h>
#include <tqobject.h>
#include <tqptrlist.h>

class KConfig;

/**This class represents a text-to-speech system.
  *@author Gunnar Schmi Dt
  */

class TextToSpeechSystem : public QObject{
   Q_OBJECT
   friend class TextToSpeechConfigurationWidget;
public:
   TextToSpeechSystem();
   ~TextToSpeechSystem();

   void readOptions (KConfig *config, const TQString &langGroup);
   void saveOptions (KConfig *config, const TQString &langGroup);

public slots:
   void speak (const TQString &text, const TQString &language);

private:
   void buildCodecList ();

   TQPtrList<TQTextCodec> *codecList;
   int codec;
   TQString ttsCommand;
   bool stdIn;
   bool useKttsd;
};

#endif
