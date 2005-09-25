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
//Added by qt3to4:
#include <QKeyEvent>

PhraseEdit::PhraseEdit(const QString &string, QWidget *parent)
 : KLineEdit (string, parent) {
}

PhraseEdit::~PhraseEdit() {
}

void PhraseEdit::keyPressEvent (QKeyEvent *e) {
   if ((e->state() & Qt::KeyboardModifierMask) == Qt::ControlModifier) {
      if (e->key() == Qt::Key_C) {
         if (!this->hasSelectedText()) {
            e->ignore();
            return;
         }
      }
      else if (e->key() == Qt::Key_Insert) {
         if (!hasSelectedText()) {
            e->ignore();
            return;
         }
      }
      else if (e->key() == Qt::Key_X) {
         if (!hasSelectedText()) {
            e->ignore();
            return;
         }
      }
   }
   else if ((e->state() & Qt::KeyboardModifierMask) == Qt::ShiftModifier) {
      if (e->key() == Qt::Key_Delete) {
         if (!hasSelectedText()) {
            e->ignore();
            return;
         }
      }
   }
   KLineEdit::keyPressEvent(e);
}
