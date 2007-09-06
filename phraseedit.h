/***************************************************************************
                          phraseedit.h  -  description
                             -------------------
    begin                : Don Sep 26 2002
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

#ifndef PHRASEEDIT_H
#define PHRASEEDIT_H

#include <klineedit.h>

#include <QtGui/QKeyEvent>

/**
 * This class extends a KLineEdit by consuming fewer unused key presses.
 *@author Gunnar Schmi Dt
 */

class PhraseEdit : public KLineEdit  {
public: 
   PhraseEdit(const QString &string, QWidget *parent);
   virtual ~PhraseEdit();

   void keyPressEvent (QKeyEvent *e);
};

#endif
