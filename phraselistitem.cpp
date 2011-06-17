/***************************************************************************
                          phraselistitem.cpp  -  description
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

#include "phraselistitem.h"
#include <tqstyle.h>
#include <tqpainter.h>

PhraseListItem::PhraseListItem (const TQString & text)
   : TQListBoxText::TQListBoxText(text) {
}

PhraseListItem::~PhraseListItem() {
}

bool PhraseListItem::drawCursor() const {
   if ((TQListBoxItem *)this != listBox()->item (listBox()->currentItem()))
      return false;
   
   for (TQListBoxItem *item = listBox()->firstItem(); item != 0; item = item->next() ) {
      if (item->isSelected())
         return true;
   }
   return false;
}

int PhraseListItem::rtti() const {
   return RTTI;
}

void PhraseListItem::paint (TQPainter *p) {
   TQListBoxText::paint (p);

   if (drawCursor()) {
      TQRect r (0, 0, listBox()->maxItemWidth(), height (listBox()));
      listBox()->tqstyle().tqdrawPrimitive (TQStyle::PE_FocusRect, p, r,
                                        listBox()->tqcolorGroup());
   }
}
