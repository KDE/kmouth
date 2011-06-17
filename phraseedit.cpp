/***************************************************************************
                          phraseedit.cpp  -  description
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

#include "phraseedit.h"

PhraseEdit::PhraseEdit(const TQString &string, TQWidget *tqparent)
 : KLineEdit (string, tqparent) {
}

PhraseEdit::~PhraseEdit() {
}

void PhraseEdit::keyPressEvent (TQKeyEvent *e) {
   if ((e->state() & TQt::KeyButtonMask) == TQt::ControlButton) {
      if (e->key() == TQt::Key_C) {
         if (!this->hasSelectedText()) {
            e->ignore();
            return;
         }
      }
      else if (e->key() == TQt::Key_Insert) {
         if (!hasSelectedText()) {
            e->ignore();
            return;
         }
      }
      else if (e->key() == TQt::Key_X) {
         if (!hasSelectedText()) {
            e->ignore();
            return;
         }
      }
   }
   else if ((e->state() & TQt::KeyButtonMask) == TQt::ShiftButton) {
      if (e->key() == TQt::Key_Delete) {
         if (!hasSelectedText()) {
            e->ignore();
            return;
         }
      }
   }
   KLineEdit::keyPressEvent(e);
}
