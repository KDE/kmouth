/***************************************************************************
                          wordcompletionwidget.h  -  description
                             -------------------
    begin                : Tue Apr 29 2003
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

#ifndef WORDCOMPLETIONWIDGET_H
#define WORDCOMPLETIONWIDGET_H

#include "wordlistui.h"
class WordCompletion;

/**This class represents a configuration widget for user preferences.
  *@author Gunnar Schmi Dt
  */

class WordCompletionWidget : public WordListUI {
   Q_OBJECT
public:
   WordCompletionWidget(WordCompletion *completion, QWidget *parent, const char *name);
   ~WordCompletionWidget();

   void ok();

private:
   WordCompletion *completion;
};

#endif
