/***************************************************************************
                          phraselistitem.h  -  description
                             -------------------
    begin                : Fre Sep 6 2002
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

#ifndef PHRASELISTITEM_H
#define PHRASELISTITEM_H

#include <q3listbox.h>

/**
 * This class represents a phrase in the list of spoken phrases. It extends
 * QListBoxText for providing support for a visible list cursor.
 * @author Gunnar Schmi Dt
 */

class PhraseListItem : public Q3ListBoxText  {
public:
   PhraseListItem (const QString & text);
   ~PhraseListItem();
   
   bool drawCursor() const;
   
   int rtti() const;
   static const int RTTI = 982734;

protected:
   void  paint( QPainter * );
};

#endif
