/***************************************************************************
                          optionsdialog.h  -  description
                             -------------------
    begin                : Don Nov 21 2002
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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <kpagedialog.h>

#include <QtCore/QObject>

#include "ui_preferencesui.h"
#include "texttospeechconfigurationwidget.h"

class QTabWidget;
class KCModule;
class WordCompletionWidget;

/**This class represents a configuration widget for user preferences.
  *@author Gunnar Schmi Dt
  */

class PreferencesWidget : public QWidget, public Ui::PreferencesUI {
   Q_OBJECT
public:
   PreferencesWidget(QWidget *parent, const char *name);
   ~PreferencesWidget();

   void readOptions (KConfig *config);
   void saveOptions (KConfig *config);

   void ok();
   void cancel();

   bool isSpeakImmediately();

private:
   bool speak;
   int save;
};

/**This class represents a configuration dialog for the options of KMouth.
  *@author Gunnar Schmi Dt
  */

class OptionsDialog : public KPageDialog {
   Q_OBJECT
public: 
   OptionsDialog(QWidget *parent);
   ~OptionsDialog();

   TextToSpeechSystem *getTTSSystem() const;

   void readOptions (KConfig *config);
   void saveOptions (KConfig *config);

   bool isSpeakImmediately();

signals:
   void configurationChanged ();
   
private slots:
   void slotCancel();
   void slotOk();
   void slotApply();

private:
   QTabWidget *tabCtl;
   TextToSpeechConfigurationWidget *commandWidget;
   PreferencesWidget *behaviourWidget;
   KCModule *kttsd;
   WordCompletionWidget *completionWidget;

   KCModule *loadKttsd ();
   void unloadKttsd ();
};

#endif
