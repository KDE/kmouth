/***************************************************************************
                          optionsdialog.cpp  -  description
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

#include "optionsdialog.h"
#include "wordcompletion/wordcompletionwidget.h"

#include "texttospeechconfigurationwidget.h"
#include "speech.h"

#include <QLayout>
#include <QLabel>
#include <QPixmap>
#include <QFile>

#include <QDialog>
#include <kcombobox.h>
#include <ktabwidget.h>
#include <klocale.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kcmodule.h>
#include <klibloader.h>
#include <QIcon>
#include <kpagewidgetmodel.h>
#include <KConfigGroup>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QVBoxLayout>

PreferencesWidget::PreferencesWidget(QWidget *parent, const char *name)
    : QWidget(parent)
{
    setObjectName(QLatin1String(name));
    setupUi(this);
    speakCombo->setCurrentIndex(1);
    speak = false;

    closeCombo->setCurrentIndex(2);
    save = 2;
}

PreferencesWidget::~PreferencesWidget()
{
}

void PreferencesWidget::cancel()
{
    if (speak)
        speakCombo->setCurrentIndex(0);
    else
        speakCombo->setCurrentIndex(1);
    closeCombo->setCurrentIndex(save);
}

void PreferencesWidget::ok()
{
    speak = speakCombo->currentIndex() == 0;
    save  = closeCombo->currentIndex();
}

void PreferencesWidget::readOptions()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "Preferences");
    if (cg.hasKey("AutomaticSpeak"))
        if (cg.readEntry("AutomaticSpeak") == QLatin1String("Yes"))
            speak = true;
        else
            speak = false;
    else
        speak = false;

    KConfigGroup cg2(KSharedConfig::openConfig() , "Notification Messages");
    if (cg2.hasKey("AutomaticSave"))
        if (cg2.readEntry("AutomaticSave") == QLatin1String("Yes"))
            save = 0;
        else
            save = 1;
    else
        save = 2;

    if (speak)
        speakCombo->setCurrentIndex(0);
    else
        speakCombo->setCurrentIndex(1);
    closeCombo->setCurrentIndex(save);
}

void PreferencesWidget::saveOptions()
{
    KConfigGroup cg(KSharedConfig::openConfig(), "Preferences");
    if (speak)
        cg.writeEntry("AutomaticSpeak", "Yes");
    else
        cg.writeEntry("AutomaticSpeak", "No");

    KConfigGroup cg2(KSharedConfig::openConfig(), "Notification Messages");
    if (save == 0)
        cg2.writeEntry("AutomaticSave", "Yes");
    else if (save == 1)
        cg2.writeEntry("AutomaticSave", "No");
    else
        cg2.deleteEntry("AutomaticSave");
}

bool PreferencesWidget::isSpeakImmediately()
{
    return speak;
}

/***************************************************************************/

OptionsDialog::OptionsDialog(QWidget *parent)
    : KPageDialog(parent)
{
    setWindowTitle(i18n("Configuration"));
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel|QDialogButtonBox::Help|QDialogButtonBox::Apply);
    QWidget *mainWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(mainWidget);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    //PORTING SCRIPT: WARNING mainLayout->addWidget(buttonBox) must be last item in layout. Please move it.
    mainLayout->addWidget(buttonBox);
    setFaceType(KPageDialog::List);
    //setHelp(QLatin1String("config-dialog"));


    //addGridPage (1, Qt::Horizontal, i18n("General Options"), QString(), iconGeneral);

    tabCtl = new KTabWidget();
    tabCtl->setObjectName(QLatin1String("general"));

    behaviourWidget = new PreferencesWidget(tabCtl, "prefPage");
//TODO PORT QT5     behaviourWidget->layout()->setMargin(QDialog::marginHint());
    tabCtl->addTab(behaviourWidget, i18n("&Preferences"));

    commandWidget = new TextToSpeechConfigurationWidget(tabCtl, "ttsTab");
//TODO PORT QT5     commandWidget->layout()->setMargin(QDialog::marginHint());
    tabCtl->addTab(commandWidget, i18n("&Text-to-Speech"));

    KPageWidgetItem *pageGeneral = new KPageWidgetItem(tabCtl, i18n("General Options"));
    pageGeneral->setHeader(i18n("General Options"));
    pageGeneral->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    addPage(pageGeneral);

    completionWidget = new WordCompletionWidget(0, "Word Completion widget");
    KPageWidgetItem *pageCompletion = new KPageWidgetItem(completionWidget, i18n("Word Completion"));
    pageCompletion->setHeader(i18n("Word Completion"));
    pageCompletion->setIcon(QIcon::fromTheme(QLatin1String("keyboard")));
    addPage(pageCompletion);

    kttsd = loadKttsd();
    if (kttsd != 0) {
        KPageWidgetItem *pageKttsd = new KPageWidgetItem(kttsd, i18n("Jovie Speech Service"));
        pageKttsd->setIcon(QIcon::fromTheme(QLatin1String("multimedia")));
        pageKttsd->setHeader(i18n("KDE Text-to-Speech Daemon Configuration"));
        addPage(pageKttsd);
    }

    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);

    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOk()));
    connect(buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()), this, SLOT(slotCancel()));
    connect(buttonBox->button(QDialogButtonBox::Apply), SIGNAL(clicked()), this, SLOT(slotApply()));
}

OptionsDialog::~OptionsDialog()
{
    unloadKttsd();
}

void OptionsDialog::slotCancel()
{
//   QDialog::slotCancel();
    commandWidget->cancel();
    behaviourWidget->cancel();
    completionWidget->load();
    if (kttsd != 0)
        kttsd->load();
}

void OptionsDialog::slotOk()
{
//   QDialog::slotOk();
    commandWidget->ok();
    behaviourWidget->ok();
    completionWidget->save();
    emit configurationChanged();
    if (kttsd != 0)
        kttsd->save();

}

void OptionsDialog::slotApply()
{
//   QDialog::slotApply();
    commandWidget->ok();
    behaviourWidget->ok();
    completionWidget->save();
    emit configurationChanged();
    if (kttsd != 0)
        kttsd->save();
}

TextToSpeechSystem *OptionsDialog::getTTSSystem() const
{
    return commandWidget->getTTSSystem();
}

void OptionsDialog::readOptions()
{
    commandWidget->readOptions(QLatin1String("TTS System"));
    behaviourWidget->readOptions();
}

void OptionsDialog::saveOptions()
{
    commandWidget->saveOptions(QLatin1String("TTS System"));
    behaviourWidget->saveOptions();
    KSharedConfig::openConfig()->sync();
}

bool OptionsDialog::isSpeakImmediately()
{
    return behaviourWidget->isSpeakImmediately();
}

KCModule *OptionsDialog::loadKttsd()
{
    KLibLoader *loader = KLibLoader::self();

    QString libname = QLatin1String("kcm_kttsd");
    KLibrary *lib = loader->library(QLatin1String(QFile::encodeName(libname)));

    if (lib == 0) {
        libname = QLatin1String("libkcm_kttsd");
        lib = loader->library(QLatin1String(QFile::encodeName(QLatin1String("libkcm_kttsd"))));
    }

    if (lib != 0) {
        QString initSym(QLatin1String("init_"));
        initSym += libname;

        if (lib->resolveFunction(QFile::encodeName(initSym))) {
            // Reuse "lib" instead of letting createInstanceFromLibrary recreate it
            KLibFactory *factory = lib->factory();
            if (factory != 0) {
                KCModule *module = factory->create<KCModule> (factory);
                if (module)
                    return module;
            }
        }

        lib->unload();
    }
    return 0;
}

void OptionsDialog::unloadKttsd()
{
    KLibLoader *loader = KLibLoader::self();
    loader->unloadLibrary(QLatin1String(QFile::encodeName(QLatin1String("libkcm_kttsd"))));
    loader->unloadLibrary(QLatin1String(QFile::encodeName(QLatin1String("kcm_kttsd"))));
}

#include "optionsdialog.moc"
