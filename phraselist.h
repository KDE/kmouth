/***************************************************************************
                          phraselist.h  -  description
                             -------------------
    begin                : Mon Aug 26 15:41:23 CEST 2002
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

#ifndef PHRASELIST_H
#define PHRASELIST_H

// include files for KDE
#include <k3listbox.h>
#include <klineedit.h>
#include <kprinter.h>
#include <kconfig.h>

// include files for Qt
#include <qwidget.h>
#include <QPushButton>
#include <kcombobox.h>
//Added by qt3to4:
#include <QKeyEvent>

class WordCompletion;

/**
 * This class represents a phrase list. It contains methods for manipulating
 * the phraselist and also methods for viewing the list.
 * The phrase list consists of an edit field for entering phrases and a list
 * box for the spoken phrases.
 * 
 * @author Gunnar Schmi Dt
 */

class PhraseList : public QWidget  {
   Q_OBJECT
public:
   PhraseList(QWidget *parent=0, const char *name=0);
   ~PhraseList();
   
   /** contains the implementation for printing functionality */
   void print(KPrinter *pPrinter);

   QStringList getListSelection();
   
   bool existListSelection();
   bool existEditSelection();

public slots:
   /** Called whenever the user wants the contents of the edit line to be spoken. */
   void speak ();

   void cut();
   void copy();
   void paste();
   
   /** Insert s into the edit field. */
   void insert (const QString &s);

   /** Called whenever the user wants the selected list entries to be spoken. */
   void speakListSelection ();

   void removeListSelection ();
   void cutListSelection ();
   void copyListSelection ();

   void save ();
   void open ();
   void open (KUrl url);
   
   void selectAllEntries ();
   void deselectAllEntries ();

   void configureCompletion();
   void saveWordCompletion();
   void saveCompletionOptions(KConfig *config);
   void readCompletionOptions(KConfig *config);

protected slots:
   void lineEntered (const QString &phrase);
   void contextMenuRequested (Q3ListBoxItem *, const QPoint &pos);
   void textChanged (const QString &s);
   void selectionChanged ();
   void keyPressEvent (QKeyEvent *e);
   void configureCompletionCombo(const QStringList &list);

private:
   K3ListBox *listBox;
   KComboBox *dictionaryCombo;
   KLineEdit *lineEdit;
   QPushButton *speakButton;
   QString line;
   WordCompletion *completion;
  
   bool isInSlot;
   
   void speakPhrase (const QString &phrase);
   void setEditLineText(const QString &s);
   void insertIntoPhraseList (const QString &phrase, bool clearEditLine);
   
   void enableMenuEntries ();
};

#endif
