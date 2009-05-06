/***************************************************************************
                          kmouth.cpp  -  description
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

// application specific includes
#include "kmouth.h"
#include "phraselist.h"
#include "phrasebook/phrasebook.h"
#include "phrasebook/phrasebookdialog.h"
#include "optionsdialog.h"
#include "configwizard.h"

// include files for Qt
#include <QtCore/QDir>
#include <QtGui/QPainter>
#include <QtGui/QPrintDialog>
#include <QtGui/QMenu>

// include files for KDE
#include <kxmlguifactory.h>
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstandardaction.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
#include <kstandardshortcut.h>
#include <kapplication.h>
#include <kdeprintdialog.h>

#define ID_STATUS_MSG 1

KMouthApp::KMouthApp(QWidget* , const char* name):KXmlGuiWindow(0)
{
   setObjectName(name);
   isConfigured = false;
   config=KGlobal::config();

   ///////////////////////////////////////////////////////////////////
   // call inits to invoke all other construction parts
   initStatusBar();
   initPhraseList();
   initActions();
   optionsDialog = new OptionsDialog(this);
   connect (optionsDialog, SIGNAL(configurationChanged ()),
            this, SLOT(slotConfigurationChanged ()));
   connect (optionsDialog, SIGNAL(configurationChanged ()),
            phraseList, SLOT(configureCompletion ()));

   phrases = new KActionCollection (static_cast<QWidget*>(this));

   readOptions();
   ConfigWizard *wizard = new ConfigWizard (this, "ConfigWizard", config.data());
   if (wizard->configurationNeeded ()) {
      if (wizard->requestConfiguration ()) {
         isConfigured = true;
         saveOptions();
         wizard->saveConfig (config.data());
         readOptions();
      }
      else
         isConfigured = false;
   }
   else
      isConfigured = true;
   delete wizard;

   if (isConfigured) {
      phraseList->configureCompletion();
   }

   ///////////////////////////////////////////////////////////////////
   // disable actions at startup
   fileSaveAs->setEnabled(false);
   filePrint->setEnabled(false);

   printer = 0;
}

KMouthApp::~KMouthApp()
{
   delete printer;
}

bool KMouthApp::configured() {
   return isConfigured;
}

void KMouthApp::initActions() {
// The "File" menu
   fileOpen = actionCollection()->addAction("file_open");
   fileOpen->setIcon(KIcon("phrasehistory_open"));
   fileOpen->setText(i18n("&Open as History..."));
   fileOpen->setShortcuts(KStandardShortcut::open());
   connect(fileOpen, SIGNAL(triggered(bool)), this, SLOT(slotFileOpen()));
   fileOpen->setToolTip(i18n("Opens an existing file as history"));
   fileOpen->setWhatsThis (i18n("Opens an existing file as history"));

   fileSaveAs = actionCollection()->addAction("file_save_as");
   fileSaveAs->setIcon(KIcon("phrasehistory_save"));
   fileSaveAs->setText(i18n("Save &History As..."));
   fileSaveAs->setShortcuts(KStandardShortcut::save());
   connect(fileSaveAs, SIGNAL(triggered(bool)), this, SLOT(slotFileSaveAs()));
   fileSaveAs->setToolTip(i18n("Saves the actual history as..."));
   fileSaveAs->setWhatsThis (i18n("Saves the actual history as..."));

   filePrint = actionCollection()->addAction("file_print");
   filePrint->setIcon(KIcon("phrasehistory_print"));
   filePrint->setText(i18n("&Print History..."));
   filePrint->setShortcuts(KStandardShortcut::print());
   connect(filePrint, SIGNAL(triggered(bool)), this, SLOT(slotFilePrint()));
   filePrint->setToolTip(i18n("Prints out the actual history"));
   filePrint->setWhatsThis (i18n("Prints out the actual history"));

   fileQuit = KStandardAction::quit(this, SLOT(slotFileQuit()), actionCollection());
   fileQuit->setToolTip(i18n("Quits the application"));
   fileQuit->setWhatsThis (i18n("Quits the application"));

// The "Edit" menu
   editCut = KStandardAction::cut(phraseList, SLOT(cut()), actionCollection());
   editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
   editCut->setWhatsThis (i18n("Cuts the selected section and puts it to the clipboard. If there is some text selected in the edit field it is placed it on the clipboard. Otherwise the selected sentences in the history (if any) are placed on the clipboard."));

   editCopy = KStandardAction::copy(phraseList, SLOT(copy()), actionCollection());
   editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
   editCopy->setWhatsThis (i18n("Copies the selected section to the clipboard. If there is some text selected in the edit field it is copied to the clipboard. Otherwise the selected sentences in the history (if any) are copied to the clipboard."));

   editPaste = KStandardAction::paste(phraseList, SLOT(paste()), actionCollection());
   editPaste->setToolTip(i18n("Pastes the clipboard contents to actual position"));
   editPaste->setWhatsThis (i18n("Pastes the clipboard contents at the current cursor position into the edit field."));

   editSpeak = actionCollection()->addAction("edit_speak");
   editSpeak->setIcon(KIcon("speak"));
   editSpeak->setText(i18nc("Start speaking", "&Speak"));
   connect(editSpeak, SIGNAL(triggered(bool)), phraseList, SLOT(speak()));
   editSpeak->setToolTip(i18n("Speaks the currently active sentence(s)"));
   editSpeak->setWhatsThis (i18n("Speaks the currently active sentence(s). If there is some text in the edit field it is spoken. Otherwise the selected sentences in the history (if any) are spoken."));

// The "Phrase book" menu
   phrasebookEdit = actionCollection()->addAction("phrasebook_edit");
   phrasebookEdit->setText(i18n("&Edit..."));
   connect(phrasebookEdit, SIGNAL(triggered(bool)), this, SLOT(slotEditPhrasebook()));

// The "Options" menu
   viewMenuBar = KStandardAction::showMenubar(this, SLOT(slotViewMenuBar()), actionCollection());
   // FIXME: Disable so it will compile.
   // viewToolBar = KStandardAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
   // viewToolBar->setToolTip(i18n("Enables/disables the toolbar"));
   // viewToolBar->setWhatsThis (i18n("Enables/disables the toolbar"));

   viewPhrasebookBar = actionCollection()->add<KToggleAction>("showPhrasebookBar");
   viewPhrasebookBar->setText(i18n("Show P&hrasebook Bar"));
   connect(viewPhrasebookBar, SIGNAL(triggered(bool)), this, SLOT(slotViewPhrasebookBar()));
   viewPhrasebookBar->setToolTip(i18n("Enables/disables the phrasebook bar"));
   viewPhrasebookBar->setWhatsThis (i18n("Enables/disables the phrasebook bar"));

   viewStatusBar = KStandardAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
   viewStatusBar->setToolTip(i18n("Enables/disables the statusbar"));
   viewStatusBar->setWhatsThis (i18n("Enables/disables the statusbar"));

   configureTTS = actionCollection()->addAction("configureTTS");
   configureTTS->setIcon(KIcon("configure"));
   configureTTS->setText(i18n("&Configure KMouth..."));
   connect(configureTTS, SIGNAL(triggered(bool)), this, SLOT(slotConfigureTTS()));
   configureTTS->setToolTip(i18n("Opens the configuration dialog"));
   configureTTS->setWhatsThis (i18n("Opens the configuration dialog"));

// The "Help" menu
   // The "Help" menu will automatically get created.

// The popup menu of the list of spoken sentences
   phraseListSpeak = actionCollection()->addAction("phraselist_speak");
   phraseListSpeak->setIcon(KIcon("speak"));
   phraseListSpeak->setText(i18n("&Speak"));
   phraseListSpeak->setToolTip(i18n("Speaks the currently selected phrases in the history"));
   connect(phraseListSpeak, SIGNAL(triggered(bool)), phraseList, SLOT(speakListSelection()));
   phraseListSpeak->setWhatsThis (i18n("Speaks the currently selected phrases in the history"));

   phraseListRemove = actionCollection()->addAction("phraselist_remove");
   phraseListRemove->setIcon(KIcon("edit-delete"));
   phraseListRemove->setText(i18n("&Delete"));
   connect(phraseListRemove, SIGNAL(triggered(bool)), phraseList, SLOT(removeListSelection()));
   phraseListRemove->setToolTip(i18n("Deletes the currently selected phrases from the history"));
   phraseListRemove->setWhatsThis (i18n("Deletes the currently selected phrases from the history"));

   phraseListCut = actionCollection()->addAction("phraselist_cut");
   phraseListCut->setIcon(KIcon("edit-cut"));
   phraseListCut->setText(i18n("Cu&t"));
   connect(phraseListCut, SIGNAL(triggered(bool)), phraseList, SLOT(cutListSelection()));
   phraseListCut->setToolTip(i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));
   phraseListCut->setWhatsThis (i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));

   phraseListCopy = actionCollection()->addAction("phraselist_copy");
   phraseListCopy->setIcon(KIcon("edit-copy"));
   phraseListCopy->setText(i18n("&Copy"));
   connect(phraseListCopy, SIGNAL(triggered(bool)), phraseList, SLOT(copyListSelection()));
   phraseListCut->setToolTip(i18n("Copies the currently selected phrases from the history to the clipboard"));
   phraseListCut->setWhatsThis (i18n("Copies the currently selected phrases from the history to the clipboard"));

   phraselistSelectAll = actionCollection()->addAction("phraselist_select_all");
   phraselistSelectAll->setText(i18n("Select &All Entries"));
   connect(phraselistSelectAll, SIGNAL(triggered(bool)), phraseList, SLOT(selectAllEntries()));
   phraselistSelectAll->setToolTip(i18n("Selects all phrases in the history"));
   phraselistSelectAll->setWhatsThis (i18n("Selects all phrases in the history"));

   phraselistDeselectAll = actionCollection()->addAction("phraselist_deselect_all");
   phraselistDeselectAll->setText(i18n("D&eselect All Entries"));
   connect(phraselistDeselectAll, SIGNAL(triggered(bool)), phraseList, SLOT(deselectAllEntries()));
   phraselistDeselectAll->setToolTip(i18n("Deselects all phrases in the history"));
   phraselistDeselectAll->setWhatsThis (i18n("Deselects all phrases in the history"));

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
  statusBar()->insertItem(i18nc("The job is done", "Ready."), ID_STATUS_MSG);
}

void KMouthApp::initPhraseList()
{
  ////////////////////////////////////////////////////////////////////
  // create the main widget here that is managed by KTMainWindow's view-region and
  // connect the widget to your document to display document contents.

   phraseList = new PhraseList(this);
   setCentralWidget(phraseList);
}

void KMouthApp::openDocumentFile(const KUrl& url)
{
  slotStatusMsg(i18n("Opening file..."));

  phraseList->open (url);
  slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::saveOptions() {
   if (isConfigured) {
      KConfigGroup cg( config, "General Options");
      cg.writeEntry("Geometry", size());
      cg.writeEntry("Show Menubar", viewMenuBar->isChecked());
      // FIXME: Toolbar disabled so it will compile.
      // cg.writeEntry("Show Toolbar", viewToolBar->isChecked());
      cg.writeEntry("Show Phrasebook Bar", viewPhrasebookBar->isChecked());
      cg.writeEntry("Show Statusbar",viewStatusBar->isChecked());
      // FIXME: KToolBar no longer has barPos() method.
      // cg.writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

      if (phraseList != 0)
         phraseList->saveCompletionOptions(config.data());
      optionsDialog->saveOptions(config.data());
      cg.changeGroup( "mainToolBar" );
      toolBar("mainToolBar")->saveSettings( cg );
      cg.changeGroup( "phrasebookBar");
      toolBar("phrasebookBar")->saveSettings( cg );
   }
}


void KMouthApp::readOptions()
{
  KConfigGroup cg( config, "General Options");

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

  QSize size=cg.readEntry("Geometry",QSize());
  if(!size.isEmpty())
  {
    resize(size);
  }

  optionsDialog->readOptions(config.data());

  toolBar("mainToolBar")->applySettings (config->group( "mainToolBar" ) );
  toolBar("phrasebookBar")->applySettings (config->group( "phrasebookBar") );

  QString standardBook = KGlobal::dirs()->findResource("appdata", "standard.phrasebook");
  if (!standardBook.isEmpty()) {
     PhraseBook book;
     book.open(KUrl::fromPathOrUrl( standardBook ));
     slotPhrasebookConfirmed(book);
  }
  if (phraseList != 0)
     phraseList->readCompletionOptions(config.data());
}

bool KMouthApp::queryClose()
{
  return true;
}

bool KMouthApp::queryExit()
{
  saveOptions();
  return true;
}

void KMouthApp::enableMenuEntries(bool existSelectedEntries, bool existDeselectedEntries) {
  bool existEntries = existSelectedEntries | existDeselectedEntries;
  fileSaveAs->setEnabled (existEntries);
  filePrint->setEnabled (existEntries);

  phraselistSelectAll->setEnabled (existDeselectedEntries);

  phraselistDeselectAll->setEnabled (existSelectedEntries);
  phraseListSpeak->setEnabled (existSelectedEntries);
  phraseListRemove->setEnabled (existSelectedEntries);
  phraseListCut->setEnabled (existSelectedEntries);
  phraseListCopy->setEnabled (existSelectedEntries);
}

/////////////////////////////////////////////////////////////////////
// SLOT IMPLEMENTATION
/////////////////////////////////////////////////////////////////////

void KMouthApp::slotFileOpen() {
   slotStatusMsg(i18n("Opening file..."));

   phraseList->open();

   slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotFileSaveAs() {
   slotStatusMsg(i18n("Saving history with a new filename..."));

   phraseList->save();

   slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

  if (printer == 0) {
    printer = new QPrinter();
  }

  QPrintDialog *printDialog = KdePrint::createPrintDialog(printer, this);

  if (printDialog->exec())
  {
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
  KMainWindow* w;
  if (!memberList().isEmpty())
  {
	for (int i = 0; i < memberList().size(); ++i)
     {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
	w = memberList().at(i);
      if(!w->close())
         break;
#ifdef __GNUC__
#warning "kde4: how remove it ?.???"
#endif
      //memberList()->removeRef(w);
    }
  }
}

void KMouthApp::slotEditPhrasebook () {
   PhraseBookDialog *phraseBookDialog = PhraseBookDialog::get();
   // As we do not know whether the we are already connected to the slot,
   // we first disconnect and then connect again.
   disconnect (phraseBookDialog, SIGNAL(phrasebookConfirmed (PhraseBook &)),
               this, SLOT(slotPhrasebookConfirmed (PhraseBook &)));
   connect (phraseBookDialog, SIGNAL(phrasebookConfirmed (PhraseBook &)),
            this, SLOT(slotPhrasebookConfirmed (PhraseBook &)));

   // As we do not know whether the phrase book edit window is already open,
   // we first open and then raise it, so that it is surely the top window.
   phraseBookDialog->show();
   phraseBookDialog->raise();
}

void KMouthApp::slotViewMenuBar() {
   slotStatusMsg(i18n("Toggling menubar..."));

   if(!viewMenuBar->isChecked())
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
  if(!viewToolBar->isChecked())
  {
    toolBar("mainToolBar")->hide();
  }
  else
  {
    toolBar("mainToolBar")->show();
  }

  slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotViewPhrasebookBar()
{
  slotStatusMsg(i18n("Toggling phrasebook bar..."));
  ///////////////////////////////////////////////////////////////////
  // turn Toolbar on or off
  if(!viewPhrasebookBar->isChecked())
  {
    toolBar("phrasebookBar")->hide();
  }
  else
  {
    toolBar("phrasebookBar")->show();
  }

  slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotViewStatusBar()
{
  slotStatusMsg(i18n("Toggle the statusbar..."));
  ///////////////////////////////////////////////////////////////////
  //turn Statusbar on or off
  if(!viewStatusBar->isChecked())
  {
    statusBar()->hide();
  }
  else
  {
    statusBar()->show();
  }

  slotStatusMsg(i18nc("The job is done", "Ready."));
}

void KMouthApp::slotConfigureTTS() {
   phraseList->saveWordCompletion();
   optionsDialog->show();
}


void KMouthApp::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clearMessage();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KMouthApp::slotPhrasebookConfirmed (PhraseBook &book) {
   QString name = "phrasebooks";
   QMenu *popup = (QMenu *)factory()->container(name, this);
   KToolBar *toolbar = toolBar ("phrasebookBar");

   delete phrases;
   phrases = new KActionCollection (actionCollection());
   book.addToGUI (popup, toolbar, phrases, this, SLOT(slotPhraseSelected (const QString &)));

   QString bookLocation = KGlobal::dirs()->saveLocation ("appdata", "/");
   if (!bookLocation.isNull() && !bookLocation.isEmpty()) {
      book.save (KUrl::fromPathOrUrl( bookLocation + "standard.phrasebook" ));
   }
}

void KMouthApp::slotConfigurationChanged()
{
   optionsDialog->saveOptions (config.data());
}

void KMouthApp::slotPhraseSelected (const QString &phrase) {
   phraseList->insert (phrase);
   if (optionsDialog->isSpeakImmediately())
      phraseList->speak ();
}

TextToSpeechSystem *KMouthApp::getTTSSystem() const {
   return optionsDialog->getTTSSystem();
}

#include "kmouth.moc"
