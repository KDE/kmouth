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

// application specific includes
#include "kmouth.h"

// include files for Qt
#include <QMenu>
#include <QMenuBar>
#include <QPrintDialog>
#include <QStandardPaths>
#include <QStatusBar>

// include files for KDE
#include <KActionCollection>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KStandardAction>
#include <KStandardShortcut>
#include <KToggleAction>
#include <KToolBar>
#include <KXMLGUIFactory>

#include "configwizard.h"
#include "optionsdialog.h"
#include "phrasebook/phrasebook.h"
#include "phrasebook/phrasebookdialog.h"
#include "phraselist.h"

KMouthApp::KMouthApp(QWidget *, const QString &name)
    : KXmlGuiWindow(nullptr)
{
    setWindowIcon(QIcon::fromTheme(QStringLiteral("kmouth")));
    setObjectName(name);
    isConfigured = false;

    ///////////////////////////////////////////////////////////////////
    // call inits to invoke all other construction parts
    initStatusBar();
    initPhraseList();
    initActions();
    optionsDialog = new OptionsDialog(this);
    connect(optionsDialog, &OptionsDialog::configurationChanged, this, &KMouthApp::slotConfigurationChanged);
    connect(optionsDialog, &OptionsDialog::configurationChanged, phraseList, &PhraseList::configureCompletion);

    phrases = new KActionCollection(static_cast<QWidget *>(this));

    readOptions();
    ConfigWizard *wizard = new ConfigWizard(this);
    if (wizard->configurationNeeded()) {
        if (wizard->requestConfiguration()) {
            isConfigured = true;
            saveOptions();
            wizard->saveConfig();
            readOptions();
        } else
            isConfigured = false;
    } else
        isConfigured = true;
    delete wizard;

    if (isConfigured) {
        phraseList->configureCompletion();
    }

    ///////////////////////////////////////////////////////////////////
    // disable actions at startup
    fileSaveAs->setEnabled(false);
    filePrint->setEnabled(false);

    printer = nullptr;
}

KMouthApp::~KMouthApp()
{
    delete printer;
}

bool KMouthApp::configured()
{
    return isConfigured;
}

void KMouthApp::initActions()
{
    // The "File" menu
    fileOpen = actionCollection()->addAction(QStringLiteral("file_open"));
    fileOpen->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    fileOpen->setText(i18n("&Open as History..."));
    actionCollection()->setDefaultShortcuts(fileOpen, KStandardShortcut::open());
    connect(fileOpen, &QAction::triggered, this, &KMouthApp::slotFileOpen);
    fileOpen->setToolTip(i18n("Opens an existing file as history"));
    fileOpen->setWhatsThis(i18n("Opens an existing file as history"));

    fileSaveAs = actionCollection()->addAction(QStringLiteral("file_save_as"));
    fileSaveAs->setIcon(QIcon::fromTheme(QStringLiteral("document-save")));
    fileSaveAs->setText(i18n("Save &History As..."));
    actionCollection()->setDefaultShortcuts(fileSaveAs, KStandardShortcut::save());
    connect(fileSaveAs, &QAction::triggered, this, &KMouthApp::slotFileSaveAs);
    fileSaveAs->setToolTip(i18n("Saves the actual history as..."));
    fileSaveAs->setWhatsThis(i18n("Saves the actual history as..."));

    filePrint = actionCollection()->addAction(QStringLiteral("file_print"));
    filePrint->setIcon(QIcon::fromTheme(QStringLiteral("document-print")));
    filePrint->setText(i18n("&Print History..."));
    actionCollection()->setDefaultShortcuts(filePrint, KStandardShortcut::print());
    connect(filePrint, &QAction::triggered, this, &KMouthApp::slotFilePrint);
    filePrint->setToolTip(i18n("Prints out the actual history"));
    filePrint->setWhatsThis(i18n("Prints out the actual history"));

    fileQuit = KStandardAction::quit(this, SLOT(slotFileQuit()), actionCollection());
    fileQuit->setToolTip(i18n("Quits the application"));
    fileQuit->setWhatsThis(i18n("Quits the application"));

    // The "Edit" menu
    editCut = KStandardAction::cut(phraseList, SLOT(cut()), actionCollection());
    editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
    editCut->setWhatsThis(
        i18n("Cuts the selected section and puts it to the clipboard. If there is some text selected in the edit field it is placed it on the clipboard. "
             "Otherwise the selected sentences in the history (if any) are placed on the clipboard."));

    editCopy = KStandardAction::copy(phraseList, SLOT(copy()), actionCollection());
    editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
    editCopy->setWhatsThis(
        i18n("Copies the selected section to the clipboard. If there is some text selected in the edit field it is copied to the clipboard. Otherwise the "
             "selected sentences in the history (if any) are copied to the clipboard."));

    editPaste = KStandardAction::paste(phraseList, SLOT(paste()), actionCollection());
    editPaste->setToolTip(i18n("Pastes the clipboard contents to current position"));
    editPaste->setWhatsThis(i18n("Pastes the clipboard contents at the current cursor position into the edit field."));

    editSpeak = actionCollection()->addAction(QStringLiteral("edit_speak"));
    editSpeak->setIcon(QIcon::fromTheme(QStringLiteral("text-speak")));
    editSpeak->setText(i18nc("Start speaking", "&Speak"));
    connect(editSpeak, &QAction::triggered, phraseList, &PhraseList::speak);
    editSpeak->setToolTip(i18n("Speaks the currently active sentence(s)"));
    editSpeak->setWhatsThis(
        i18n("Speaks the currently active sentence(s). If there is some text in the edit field it is spoken. Otherwise the selected sentences in the history "
             "(if any) are spoken."));

    // The "Phrase book" menu
    phrasebookEdit = actionCollection()->addAction(QStringLiteral("phrasebook_edit"));
    phrasebookEdit->setText(i18n("&Edit..."));
    connect(phrasebookEdit, &QAction::triggered, this, &KMouthApp::slotEditPhrasebook);

    // The "Options" menu
    viewMenuBar = KStandardAction::showMenubar(this, SLOT(slotViewMenuBar()), actionCollection());
    // FIXME: Disable so it will compile.
    // viewToolBar = KStandardAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
    // viewToolBar->setToolTip(i18n("Enables/disables the toolbar"));
    // viewToolBar->setWhatsThis (i18n("Enables/disables the toolbar"));

    viewPhrasebookBar = actionCollection()->add<KToggleAction>(QStringLiteral("showPhrasebookBar"));
    viewPhrasebookBar->setText(i18n("Show P&hrasebook Bar"));
    connect(viewPhrasebookBar, &QAction::triggered, this, &KMouthApp::slotViewPhrasebookBar);
    viewPhrasebookBar->setToolTip(i18n("Enables/disables the phrasebook bar"));
    viewPhrasebookBar->setWhatsThis(i18n("Enables/disables the phrasebook bar"));

    viewStatusBar = KStandardAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
    viewStatusBar->setToolTip(i18n("Enables/disables the statusbar"));
    viewStatusBar->setWhatsThis(i18n("Enables/disables the statusbar"));

    configureTTS = actionCollection()->addAction(QStringLiteral("configureTTS"));
    configureTTS->setIcon(QIcon::fromTheme(QStringLiteral("configure")));
    configureTTS->setText(i18n("&Configure KMouth..."));
    connect(configureTTS, &QAction::triggered, this, &KMouthApp::slotConfigureTTS);
    configureTTS->setToolTip(i18n("Opens the configuration dialog"));
    configureTTS->setWhatsThis(i18n("Opens the configuration dialog"));

    // The "Help" menu
    // The "Help" menu will automatically get created.

    // The popup menu of the list of spoken sentences
    phraseListSpeak = actionCollection()->addAction(QStringLiteral("phraselist_speak"));
    phraseListSpeak->setIcon(QIcon::fromTheme(QStringLiteral("text-speak")));
    phraseListSpeak->setText(i18n("&Speak"));
    phraseListSpeak->setToolTip(i18n("Speaks the currently selected phrases in the history"));
    connect(phraseListSpeak, &QAction::triggered, phraseList, &PhraseList::speakListSelection);
    phraseListSpeak->setWhatsThis(i18n("Speaks the currently selected phrases in the history"));

    phraseListRemove = actionCollection()->addAction(QStringLiteral("phraselist_remove"));
    phraseListRemove->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    phraseListRemove->setText(i18n("&Delete"));
    connect(phraseListRemove, &QAction::triggered, phraseList, &PhraseList::removeListSelection);
    phraseListRemove->setToolTip(i18n("Deletes the currently selected phrases from the history"));
    phraseListRemove->setWhatsThis(i18n("Deletes the currently selected phrases from the history"));

    phraseListCut = actionCollection()->addAction(QStringLiteral("phraselist_cut"));
    phraseListCut->setIcon(QIcon::fromTheme(QStringLiteral("edit-cut")));
    phraseListCut->setText(i18n("Cu&t"));
    connect(phraseListCut, &QAction::triggered, phraseList, &PhraseList::cutListSelection);
    phraseListCut->setToolTip(i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));
    phraseListCut->setWhatsThis(i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));

    phraseListCopy = actionCollection()->addAction(QStringLiteral("phraselist_copy"));
    phraseListCopy->setIcon(QIcon::fromTheme(QStringLiteral("edit-copy")));
    phraseListCopy->setText(i18n("&Copy"));
    connect(phraseListCopy, &QAction::triggered, phraseList, &PhraseList::copyListSelection);
    phraseListCut->setToolTip(i18n("Copies the currently selected phrases from the history to the clipboard"));
    phraseListCut->setWhatsThis(i18n("Copies the currently selected phrases from the history to the clipboard"));

    phraselistSelectAll = actionCollection()->addAction(QStringLiteral("phraselist_select_all"));
    phraselistSelectAll->setText(i18n("Select &All Entries"));
    connect(phraselistSelectAll, &QAction::triggered, phraseList, &PhraseList::selectAllEntries);
    phraselistSelectAll->setToolTip(i18n("Selects all phrases in the history"));
    phraselistSelectAll->setWhatsThis(i18n("Selects all phrases in the history"));

    phraselistDeselectAll = actionCollection()->addAction(QStringLiteral("phraselist_deselect_all"));
    phraselistDeselectAll->setText(i18n("D&eselect All Entries"));
    connect(phraselistDeselectAll, &QAction::triggered, phraseList, &PhraseList::deselectAllEntries);
    phraselistDeselectAll->setToolTip(i18n("Deselects all phrases in the history"));
    phraselistDeselectAll->setWhatsThis(i18n("Deselects all phrases in the history"));

    // The popup menu of the edit field
    // The popup menu of the edit field will automatically get created.

    // use the absolute path to your kmouthui.rc file for testing purpose in createGUI();
    createGUI();
}

void KMouthApp::initStatusBar()
{
    ///////////////////////////////////////////////////////////////////
    // STATUSBAR
    // TODO: add your own items you need for displaying current application status.
    m_statusLabel = new QLabel(i18nc("The job is done", "Ready."));
    statusBar()->addPermanentWidget(m_statusLabel);
}

void KMouthApp::initPhraseList()
{
    ////////////////////////////////////////////////////////////////////
    // create the main widget here that is managed by KTMainWindow's view-region and
    // connect the widget to your document to display document contents.

    phraseList = new PhraseList(this);
    setCentralWidget(phraseList);
}

void KMouthApp::openDocumentFile(const QUrl &url)
{
    slotStatusMsg(i18n("Opening file..."));

    phraseList->open(url);
    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::saveOptions()
{
    if (isConfigured) {
        KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("General Options"));
        cg.writeEntry("Geometry", size());
        cg.writeEntry("Show Menubar", viewMenuBar->isChecked());
        // FIXME: Toolbar disabled so it will compile.
        // cg.writeEntry("Show Toolbar", viewToolBar->isChecked());
        cg.writeEntry("Show Phrasebook Bar", viewPhrasebookBar->isChecked());
        cg.writeEntry("Show Statusbar", viewStatusBar->isChecked());
        // FIXME: KToolBar no longer has barPos() method.
        // cg.writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

        if (phraseList != nullptr)
            phraseList->saveCompletionOptions();
        optionsDialog->saveOptions();
        KConfigGroup cg2(KSharedConfig::openConfig(), QStringLiteral("mainToolBar"));
        toolBar(QStringLiteral("mainToolBar"))->saveSettings(cg2);
        KConfigGroup cg3(KSharedConfig::openConfig(), QStringLiteral("phrasebookBar"));
        toolBar(QStringLiteral("phrasebookBar"))->saveSettings(cg);
    }
}

void KMouthApp::readOptions()
{
    KConfigGroup cg(KSharedConfig::openConfig(), QStringLiteral("General Options"));

    // bar status settings
    bool bViewMenubar = cg.readEntry("Show Menubar", true);
    viewMenuBar->setChecked(bViewMenubar);
    slotViewMenuBar();

    // FIXME: Toolbar disabled so it will compile.
    // bool bViewToolbar = cg.readEntry("Show Toolbar", QVariant(true)).toBool();
    // viewToolBar->setChecked(bViewToolbar);
    // slotViewToolBar();

    bool bViewPhrasebookbar = cg.readEntry("Show Phrasebook Bar", true);
    viewPhrasebookBar->setChecked(bViewPhrasebookbar);

    bool bViewStatusbar = cg.readEntry("Show Statusbar", true);
    viewStatusBar->setChecked(bViewStatusbar);
    slotViewStatusBar();

    // bar position settings
    // FIXME:
    // KToolBar::BarPosition toolBarPos;
    // toolBarPos=(KToolBar::BarPosition) cg.readEntry("ToolBarPos", int(KToolBar::Top));
    // toolBar("mainToolBar")->setBarPos(toolBarPos);

    QSize size = cg.readEntry("Geometry", QSize());
    if (!size.isEmpty()) {
        resize(size);
    }

    optionsDialog->readOptions();

    toolBar(QStringLiteral("mainToolBar"))->applySettings(KSharedConfig::openConfig()->group(QStringLiteral("mainToolBar")));
    toolBar(QStringLiteral("phrasebookBar"))->applySettings(KSharedConfig::openConfig()->group(QStringLiteral("phrasebookBar")));

    slotPhrasebookConfirmed();
    if (phraseList != nullptr)
        phraseList->readCompletionOptions();
}

bool KMouthApp::queryClose()
{
    saveOptions();
    return true;
}

void KMouthApp::enableMenuEntries(bool existSelectedEntries, bool existDeselectedEntries)
{
    bool existEntries = existSelectedEntries | existDeselectedEntries;
    fileSaveAs->setEnabled(existEntries);
    filePrint->setEnabled(existEntries);

    phraselistSelectAll->setEnabled(existDeselectedEntries);

    phraselistDeselectAll->setEnabled(existSelectedEntries);
    phraseListSpeak->setEnabled(existSelectedEntries);
    phraseListRemove->setEnabled(existSelectedEntries);
    phraseListCut->setEnabled(existSelectedEntries);
    phraseListCopy->setEnabled(existSelectedEntries);
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KMouthApp::slotFileOpen()
{
    slotStatusMsg(i18n("Opening file..."));

    phraseList->open();

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotFileSaveAs()
{
    slotStatusMsg(i18n("Saving history with a new filename..."));

    phraseList->save();

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotFilePrint()
{
    slotStatusMsg(i18n("Printing..."));

    if (printer == nullptr) {
        printer = new QPrinter();
    }

    QPrintDialog *printDialog = new QPrintDialog(printer, nullptr);

    if (printDialog->exec()) {
        phraseList->print(printer);
    }

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotFileQuit()
{
    slotStatusMsg(i18nc("Shutting down the application", "Exiting..."));
    saveOptions();
    // close the first window, the list makes the next one the first again.
    // This ensures that queryClose() is called on each window to ask for closing
    if (!memberList().isEmpty()) {
        for (int i = 0; i < memberList().size(); ++i) {
            // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
            // the window and the application stay open.
            KMainWindow *w = memberList().at(i);
            if (!w->close())
                break;
#ifdef __GNUC__
#warning "kde4: how remove it ?.???"
#endif
            // memberList()->removeRef(w);
        }
    }
}

void KMouthApp::slotEditPhrasebook()
{
    PhraseBookDialog *phraseBookDialog = PhraseBookDialog::get();
    // As we do not know whether the we are already connected to the slot,
    // we first disconnect and then connect again.
    disconnect(phraseBookDialog, &PhraseBookDialog::phrasebookConfirmed, this, &KMouthApp::slotPhrasebookConfirmed);
    connect(phraseBookDialog, &PhraseBookDialog::phrasebookConfirmed, this, &KMouthApp::slotPhrasebookConfirmed);

    // As we do not know whether the phrase book edit window is already open,
    // we first open and then raise it, so that it is surely the top window.
    phraseBookDialog->show();
    phraseBookDialog->raise();
}

void KMouthApp::slotViewMenuBar()
{
    slotStatusMsg(i18n("Toggling menubar..."));

    if (!viewMenuBar->isChecked())
        menuBar()->hide();
    else
        menuBar()->show();

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotViewToolBar()
{
    slotStatusMsg(i18n("Toggling toolbar..."));
    ///////////////////////////////////////////////////////////////////
    // turn Toolbar on or off
    if (!viewToolBar->isChecked()) {
        toolBar(QStringLiteral("mainToolBar"))->hide();
    } else {
        toolBar(QStringLiteral("mainToolBar"))->show();
    }

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotViewPhrasebookBar()
{
    slotStatusMsg(i18n("Toggling phrasebook bar..."));
    ///////////////////////////////////////////////////////////////////
    // turn Toolbar on or off
    if (!viewPhrasebookBar->isChecked()) {
        toolBar(QStringLiteral("phrasebookBar"))->hide();
    } else {
        toolBar(QStringLiteral("phrasebookBar"))->show();
    }

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotViewStatusBar()
{
    slotStatusMsg(i18n("Toggle the statusbar..."));
    ///////////////////////////////////////////////////////////////////
    // turn Statusbar on or off
    if (!viewStatusBar->isChecked()) {
        statusBar()->hide();
    } else {
        statusBar()->show();
    }

    slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotConfigureTTS()
{
    phraseList->saveWordCompletion();
    optionsDialog->show();
}

void KMouthApp::slotStatusMsg(const QString &text)
{
    ///////////////////////////////////////////////////////////////////
    // change status message permanently
    m_statusLabel->setText(text);
}

void KMouthApp::slotPhrasebookConfirmed()
{
    QString standardBook = QStandardPaths::locate(QStandardPaths::AppDataLocation, QStringLiteral("standard.phrasebook"));
    if (!standardBook.isEmpty()) {
        PhraseBook book;
        book.open(QUrl::fromLocalFile(standardBook));
        QString name = QStringLiteral("phrasebooks");
        QMenu *popup = (QMenu *)factory()->container(name, this);
        KToolBar *toolbar = toolBar(QStringLiteral("phrasebookBar"));

        delete phrases;
        phrases = new KActionCollection(actionCollection());
        book.addToGUI(popup, toolbar, phrases, this, SLOT(slotPhraseSelected(QString)));
    }
}

void KMouthApp::slotConfigurationChanged()
{
    optionsDialog->saveOptions();
}

void KMouthApp::slotPhraseSelected(const QString &phrase)
{
    phraseList->insert(phrase);
    if (optionsDialog->isSpeakImmediately())
        phraseList->speak();
}

TextToSpeechSystem *KMouthApp::getTTSSystem() const
{
    return optionsDialog->getTTSSystem();
}

#include "moc_kmouth.cpp"
