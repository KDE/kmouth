/***************************************************************************
 *   Copyright (C) 2002 by Gunnar Schmi Dt <kmouth@schmi-dt.de             *
 *             (C) 2015 by Jeremy Whiting <jpwhiting@kde.org>              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 ***************************************************************************/

#ifndef DICTIONARYCREATIONWIZARD_H
#define DICTIONARYCREATIONWIZARD_H

#include <QList>
#include <QMap>
#include <QWizard>

#include "ui_creationsourceui.h"
#include "ui_creationsourcedetailsui.h"
#include "ui_kdedocsourceui.h"

class CompletionWizardWidget;
class KComboBox;
class MergeWidget;
class QScrollArea;
class QSpinBox;
class QTextCodec;

class CreationSourceWidget : public QWizardPage, public Ui::CreationSourceUI
{
    Q_OBJECT
public:
    CreationSourceWidget(QWidget *parent, const char *name)
        : QWizardPage(parent)
    {
        setupUi(this);
        setObjectName(QLatin1String(name));
        connect(emptyButton, &QAbstractButton::toggled, this, &CreationSourceWidget::emptyToggled);
    }

    int nextId() const Q_DECL_OVERRIDE;
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
    int nextId() const Q_DECL_OVERRIDE
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

        ooDictURL->setFilter(QStringLiteral("*.dic"));
    }
    int nextId() const Q_DECL_OVERRIDE
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
    QHash<QString, QSpinBox*> weights;
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

    void ok();
};

#endif
