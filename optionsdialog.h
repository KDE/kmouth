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

#include <qobject.h>
#include "preferencesui.h"
#include "texttospeechconfigurationwidget.h"
class KTabCtl;

/**This class represents a configuration widget for user preferences.
  *@author Gunnar Schmi Dt
  */

class PreferencesWidget : public PreferencesUI {
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

class OptionsDialog : public KDialogBase  {
   Q_OBJECT
public: 
   OptionsDialog(QWidget *parent);
   ~OptionsDialog();

   void slotCancel();
   void slotOk();
   void slotApply();

   TextToSpeechSystem *getTTSSystem() const;

   void readOptions (KConfig *config);
   void saveOptions (KConfig *config);

   bool isSpeakImmediately();

signals:
   void configurationChanged ();

private:
   KTabCtl *tabCtl;
   TextToSpeechConfigurationWidget *commandWidget;
   PreferencesWidget *behaviourWidget;
};

#endif

/*
 * $Log$
 * Revision 1.3  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.2  2002/11/22 08:48:34  gunnar
 * Implemented functionality that belongs to the new options in the options dialog
 *
 * Revision 1.1  2002/11/21 21:33:26  gunnar
 * Extended parameter dialog and added wizard for the first start
 *
 * Revision 1.3  2002/11/04 16:38:42  gunnar
 * Incorporated changes for version 0.5.1 into head branch
 *
 * Revision 1.2.2.1  2002/11/04 15:36:37  gunnar
 * combo box for character encoding added
 *
 * Revision 1.2  2002/09/08 19:29:42  gunnar
 * Configuration dialog improved
 *
 * Revision 1.1  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 */
