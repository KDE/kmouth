/***************************************************************************
                          wordcompletionwidget.cpp  -  description
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

#include <qcheckbox.h>
#include <qlineedit.h>

#include <kurlrequester.h>
#include <klocale.h>
#include <kglobal.h>

#include "wordcompletionwidget.h"
#include "wordcompletion.h"

WordCompletionWidget::WordCompletionWidget (WordCompletion *completion, QWidget *parent, const char *name)
   : WordListUI (parent, name)
{
   this->completion = completion;
   QStringList languages = KGlobal::locale()->languageList();
   if (languages.count() > 0) {
      if (languages[0].isNull() || languages[0].isEmpty())
         languageEdit->setText(languages[languages.count()-1]);
      else
         languageEdit->setText(languages[0]);
   }
   else
      languageEdit->setText("en");
   dictionaryURL->setFilter ("*.dic");
}

WordCompletionWidget::~WordCompletionWidget() {
}

void WordCompletionWidget::ok() {
   if (dictionaryCheckBox->isChecked())
      completion->configure("dictionary.txt", languageEdit->text(), dictionaryURL->url());
   else
      completion->configure("dictionary.txt", languageEdit->text(), QString::null);
}

#include "wordcompletionwidget.moc"
