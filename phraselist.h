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

// $Id$

#ifndef PHRASELIST_H
#define PHRASELIST_H

// include files for KDE
#include <klistbox.h>
#include <klineedit.h>
#include <kprinter.h>

// include files for Qt
#include <qwidget.h>
#include <qpushbutton.h>

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
   void open (KURL url);
   
   void selectAllEntries ();
   void deselectAllEntries ();

protected slots:
   void lineEntered (const QString &phrase);
   void contextMenuRequested (QListBoxItem *, const QPoint &pos);
   void textChanged (const QString &s);
   void selectionChanged ();
   void keyPressEvent (QKeyEvent *e);

private:
   KListBox *listBox;
   KLineEdit *lineEdit;
   QPushButton *speakButton;
   QString line;
  
   bool isInSlot;
   
   void speakPhrase (const QString &phrase);
   void setEditLineText(const QString &s);
   void insertIntoPhraseList (const QString &phrase, bool clearEditLine);
   
   void enableMenuEntries ();
};

#endif

/*
 * $Log$
 * Revision 1.8  2003/01/12 11:37:05  gunnar
 * Improved format list of file selectors / several small changes
 *
 * Revision 1.7  2002/09/26 17:10:46  gunnar
 * Several small changes
 *
 * Revision 1.6  2002/09/18 09:30:40  gunnar
 * A nuber of small changes
 *
 * Revision 1.5  2002/09/13 11:51:25  gunnar
 * Added printing support
 *
 * Revision 1.4  2002/09/13 09:40:37  gunnar
 * Added support for opening a file as history
 *
 * Revision 1.3  2002/09/12 15:28:09  gunnar
 * Loading (into the edit line) and saving (the phrase list) implemented
 *
 * Revision 1.2  2002/09/11 19:03:24  gunnar
 * Implemented the popup actions
 *
 * Revision 1.1  2002/09/11 16:57:35  gunnar
 * added context menu, and moved the contents of KMouthView and KMouthDoc into a new class PhraseList
 *
 * Log: kmouthview.h,v
 * Revision 1.6  2002/09/07 17:29:24  gunnar
 * Moved the speak button next to the edit line
 *
 * Revision 1.5  2002/09/06 13:34:03  gunnar
 * fixed undo bug and started to develop support for selection of multiple phrases
 *
 * Revision 1.4  2002/09/01 08:32:54  gunnar
 * toolbar icon and application icon added
 *
 * Revision 1.3  2002/08/29 09:14:12  gunnar
 * Added functionality for scrolling through the list of spoken sentences
 *
 * Revision 1.2  2002/08/28 19:20:04  gunnar
 * Added an edit field and a list of entered lines
 *
 * Revision 1.1.1.1  2002/08/26 14:09:49  gunnar
 * New project started
 *
 */
