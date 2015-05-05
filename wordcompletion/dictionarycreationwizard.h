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
#include <QScrollArea>
#include <QWizard>

#include <kcombobox.h>
#include <knuminput.h>
#include <KConfig>

#include "ui_creationsourceui.h"
#include "ui_creationsourcedetailsui.h"
#include "ui_kdedocsourceui.h"

class CompletionWizardWidget;
class QTextCodec;
class KComboBox;
class MergeWidget;

class CreationSourceWidget : public QWizardPage, public Ui::CreationSourceUI
{
    Q_OBJECT
public:
    CreationSourceWidget(QWidget *parent, const char *name)
        : QWizardPage(parent)
    {
        setupUi(this);
        setObjectName(QLatin1String(name));
        connect(emptyButton, SIGNAL(toggled(bool)), this, SLOT(emptyToggled(bool)));
    }

    virtual int nextId() const;
private slots:
    void emptyToggled(bool checked);
};

class CreationSourceDetailsWidget : public QWizardPage, public Ui::CreationSourceDetailsUI
{
    Q_OBJECT
public:
    CreationSourceDetailsWidget(QWidget *parent, const char *name)
        : QWizardPage(parent)
    {
        setupUi(this);
        setObjectName(QLatin1String(name));
    }
    virtual int nextId() const
    {
        return -1;
    }
};

class KDEDocSourceWidget : public QWizardPage, public Ui::KDEDocSourceUI
{
    Q_OBJECT
public:
    KDEDocSourceWidget(QWidget *parent, const char *name)
        : QWizardPage(parent)
    {
        setupUi(this);
        setObjectName(QLatin1String(name));
        languageButton->showLanguageCodes(true);
        languageButton->loadAllLanguages();

        ooDictURL->setFilter(QLatin1String("*.dic"));
    }
    virtual int nextId() const
    {
        return -1;
    }
};

/**
 * This class represents a wizard that is used in order to gather all
 * necessary information for creating a new dictionary for the word
 * completion.
 */
class DictionaryCreationWizard : public QWizard
{
    Q_OBJECT
public:
    DictionaryCreationWizard(QWidget *parent, const char *name,
                             const QStringList &dictionaryNames,
                             const QStringList &dictionaryFiles,
                             const QStringList &dictionaryLanguages);
    virtual ~DictionaryCreationWizard();

    QString createDictionary();
    QString name();
    QString language();

    enum Pages {
        CreationSourcePage,
        FilePage,
        DirPage,
        KDEDocPage,
        MergePage
    };

private:
    void buildCodecList();
    void buildCodecCombo(KComboBox *combo);

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
class MergeWidget : public QWizardPage
{
    Q_OBJECT
public:
    MergeWidget(QWidget *parent, const char *name,
                const QStringList &dictionaryNames,
                const QStringList &dictionaryFiles,
                const QStringList &dictionaryLanguages);
    ~MergeWidget();

    QMap <QString, int> mergeParameters();
    QString language();

private:
    QScrollArea *scrollArea;
    QHash<QString, QCheckBox*> dictionaries;
    QHash<QString, KIntNumInput*> weights;
    QMap<QString, QString> languages;
};

/**
 * This class represents a widget for creating an initial dictionary from the
 * KDE documentation.
 * @author Gunnar Schmi Dt
 */
class CompletionWizardWidget : public QWizardPage, public Ui::KDEDocSourceUI
{
    Q_OBJECT
    friend class ConfigWizard;
public:
    CompletionWizardWidget(QWidget *parent, const char *name);
    ~CompletionWizardWidget();

    void ok(KConfig *config);
};

#endif
