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

#include <QtCore/QList>
#include <QtCore/QMap>
#include <Qt3Support/Q3Dict>

#include <k3wizard.h>
#include <knuminput.h>

#include "ui_creationsourceui.h"
#include "ui_creationsourcedetailsui.h"
#include "ui_kdedocsourceui.h"

class CompletionWizardWidget;
class QTextCodec;
class QComboBox;
class MergeWidget;

class CreationSourceDetailsWidget : public QWidget, public Ui::CreationSourceDetailsUI {
   Q_OBJECT
public:
   CreationSourceDetailsWidget(QWidget *parent, const char *name)
      : QWidget(parent) {
      setupUi(this);
      setObjectName(name);
   }
};

class CreationSourceWidget : public QWidget, public Ui::CreationSourceUI {
   Q_OBJECT
public:
   CreationSourceWidget(QWidget *parent, const char *name)
      : QWidget(parent) {
      setupUi(this);
      setObjectName(name);
   }
};

class KDEDocSourceWidget : public QWidget, public Ui::KDEDocSourceUI {
   Q_OBJECT
public:
   KDEDocSourceWidget(QWidget *parent, const char *name)
      : QWidget(parent) {
      setupUi(this);
      setObjectName(name);
   }
};

/**
 * This class represents a wizard that is used in order to gather all
 * necessary information for creating a new dictionary for the word
 * completion.
 */
class DictionaryCreationWizard : public K3Wizard {
   Q_OBJECT
public:
   DictionaryCreationWizard (QWidget *parent, const char *name,
                             QStringList dictionaryNames,
                             QStringList dictionaryFiles,
                             QStringList dictionaryLanguages);
   ~DictionaryCreationWizard();

   QString createDictionary();
   QString name();
   QString language();

private slots:
   void calculateAppropriate (bool);

private:
   void buildCodecList ();
   void buildCodecCombo (QComboBox *combo);

   CreationSourceWidget *creationSource;
   CreationSourceDetailsWidget *fileWidget;
   CreationSourceDetailsWidget *dirWidget;
   KDEDocSourceWidget *kdeDocWidget;
   MergeWidget *mergeWidget;

   QList<QTextCodec*> *codecList;
};

/**
 * This class represents a widget for creating an initial dictionary from the
 * KDE documentation.
 * @author Gunnar Schmi Dt
 */
class MergeWidget : public Q3ScrollView {
   Q_OBJECT
public:
   MergeWidget(K3Wizard *parent, const char *name,
               QStringList dictionaryNames,
               QStringList dictionaryFiles,
               QStringList dictionaryLanguages);
   ~MergeWidget();

   QMap <QString, int> mergeParameters ();
   QString language ();

private:
   Q3Dict<QCheckBox> dictionaries;
   Q3Dict<KIntNumInput> weights;
   QMap<QString,QString> languages;
};

/**
 * This class represents a widget for creating an initial dictionary from the
 * KDE documentation.
 * @author Gunnar Schmi Dt
 */
class CompletionWizardWidget : public QWidget, public Ui::KDEDocSourceUI {
   Q_OBJECT
   friend class ConfigWizard;
public:
   CompletionWizardWidget(K3Wizard *parent, const char *name);
   ~CompletionWizardWidget();

   void ok (KConfig *config);
};

#endif
