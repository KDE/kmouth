/***************************************************************************
                          dictionarycreationwizard.h  -  description
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

#ifndef DICTIONARYCREATIONWIZARD_H
#define DICTIONARYCREATIONWIZARD_H

#include <tqptrlist.h>
#include <tqmap.h>

#include <kwizard.h>
#include <knuminput.h>
#include "kdedocsourceui.h"

class CreationSourceUI;
class CompletionWizardWidget;
class CreationSourceDetailsUI;
class KDEDocSourceUI;
class TQTextCodec;
class TQComboBox;
class MergeWidget;

/**
 * This class represents a wizard that is used in order to gather all
 * necessary information for creating a new dictionary for the word
 * completion.
 */
class DictionaryCreationWizard : public KWizard {
   Q_OBJECT
public:
   DictionaryCreationWizard (TQWidget *parent, const char *name,
                             TQStringList dictionaryNames,
                             TQStringList dictionaryFiles,
                             TQStringList dictionaryLanguages);
   ~DictionaryCreationWizard();

   TQString createDictionary();
   TQString name();
   TQString language();

private slots:
   void calculateAppropriate (bool);

private:
   void buildCodecList ();
   void buildCodecCombo (TQComboBox *combo);

   CreationSourceUI *creationSource;
   CreationSourceDetailsUI *fileWidget;
   CreationSourceDetailsUI *dirWidget;
   KDEDocSourceUI *kdeDocWidget;
   MergeWidget *mergeWidget;

   TQPtrList<TQTextCodec> *codecList;
};

/**
 * This class represents a widget for creating an initial dictionary from the
 * KDE documentation.
 * @author Gunnar Schmi Dt
 */
class MergeWidget : public TQScrollView {
   Q_OBJECT
public:
   MergeWidget(KWizard *parent, const char *name,
               TQStringList dictionaryNames,
               TQStringList dictionaryFiles,
               TQStringList dictionaryLanguages);
   ~MergeWidget();

   TQMap <TQString, int> mergeParameters ();
   TQString language ();

private:
   TQDict<TQCheckBox> dictionaries;
   TQDict<KIntNumInput> weights;
   TQMap<TQString,TQString> languages;
};

/**
 * This class represents a widget for creating an initial dictionary from the
 * KDE documentation.
 * @author Gunnar Schmi Dt
 */
class CompletionWizardWidget : public KDEDocSourceUI {
   Q_OBJECT
   friend class ConfigWizard;
public:
   CompletionWizardWidget(KWizard *parent, const char *name);
   ~CompletionWizardWidget();

   void ok (KConfig *config);
};

#endif
