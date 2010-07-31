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

#include "wordcompletionui.h"
class WordCompletion;
class KLanguageButton;

/**
 * This class represents a configuration widget for managing dictionaries.
 * @author Gunnar Schmi Dt
 */
class WordCompletionWidget : public WordCompletionUI {
   Q_OBJECT
public:
   WordCompletionWidget(TQWidget *parent, const char *name);
   ~WordCompletionWidget();

   /**
    * This method is invoked whenever the widget should read its configuration
    * from a config file and update the user interface.
    */
   void load();

   /**
    * This function gets called when the user wants to save the settings in 
    * the user interface, updating the config files.
    */
   void save();

signals:
    void changed (bool);

private slots:
   void addDictionary();
   void deleteDictionary();
   void moveUp();
   void moveDown();
   void exportDictionary();

   void selectionChanged();
   void nameChanged (const TQString &text);
   void languageSelected (int);

   /**
    * This slot is used to emit the signal changed when any widget changes
    * the configuration 
    */
   void configChanged() {
      emit changed(true);
   };

private:
   /**
    * Object holding all the configuration
    */
   KConfig *config;
   TQStringList newDictionaryFiles;
   TQStringList removedDictionaryFiles;
};

#endif
