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
#include <qstyle.h>
#include <qpainter.h>

PhraseListItem::PhraseListItem (const QString & text)
   : Q3ListBoxText(text) {
}

PhraseListItem::~PhraseListItem() {
}

bool PhraseListItem::drawCursor() const {
   if ((Q3ListBoxItem *)this != listBox()->item (listBox()->currentItem()))
      return false;
   
   for (Q3ListBoxItem *item = listBox()->firstItem(); item != 0; item = item->next() ) {
      if (item->isSelected())
         return true;
   }
   return false;
}

int PhraseListItem::rtti() const {
   return RTTI;
}

void PhraseListItem::paint (QPainter *p) {
   Q3ListBoxText::paint (p);

   if (drawCursor()) {
      QRect r (0, 0, listBox()->maxItemWidth(), height (listBox()));
      listBox()->style()->drawPrimitive (QStyle::PE_FocusRect, p, r,
                                        listBox()->colorGroup());
   }
}
