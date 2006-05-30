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

// include files for QT
#include <QDir>
#include <QPainter>
//Added by qt3to4:
#include <QMenu>
#include <kxmlguifactory.h>
// include files for KDE
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kstdaccel.h>
#include <kprinter.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kglobal.h>
#include <ktoolbar.h>
#include <kactioncollection.h>
#include <kactionmenu.h>
#include <ktoggleaction.h>
// application specific includes
#include "kmouth.h"
#include "phraselist.h"
#include "phrasebook/phrasebook.h"
#include "phrasebook/phrasebookdialog.h"
#include "optionsdialog.h"
#include "configwizard.h"

#define ID_STATUS_MSG 1

KMouthApp::KMouthApp(QWidget* , const char* name):KMainWindow(0, name)
{
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
   ConfigWizard *wizard = new ConfigWizard (this, "ConfigWizard", config);
   if (wizard->configurationNeeded ()) {
      if (wizard->requestConfiguration ()) {
         isConfigured = true;
         saveOptions();
         wizard->saveConfig (config);
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
}

KMouthApp::~KMouthApp()
{

}

bool KMouthApp::configured() {
   return isConfigured;
}

void KMouthApp::initActions() {
// The "File" menu
   fileOpen = new KAction(KIcon("phrasehistory_open"), i18n("&Open as History..."), actionCollection(),"file_open");
   fileOpen->setShortcut(KStdAccel::shortcut(KStdAccel::Open));
   connect(fileOpen, SIGNAL(triggered(bool)), this, SLOT(slotFileOpen()));
   fileOpen->setToolTip(i18n("Opens an existing file as history"));
   fileOpen->setWhatsThis (i18n("Opens an existing file as history"));

   fileSaveAs = new KAction(KIcon("phrasehistory_save"), i18n("Save &History As..."), actionCollection(),"file_save_as");
   fileSaveAs->setShortcut(KStdAccel::shortcut(KStdAccel::Save));
   connect(fileSaveAs, SIGNAL(triggered(bool)), this, SLOT(slotFileSaveAs()));
   fileSaveAs->setToolTip(i18n("Saves the actual history as..."));
   fileSaveAs->setWhatsThis (i18n("Saves the actual history as..."));

   filePrint = new KAction(KIcon("phrasehistory_print"), i18n("&Print History..."), actionCollection(),"file_print");
   filePrint->setShortcut(KStdAccel::shortcut(KStdAccel::Print));
   connect(filePrint, SIGNAL(triggered(bool)), this, SLOT(slotFilePrint()));
   filePrint->setToolTip(i18n("Prints out the actual history"));
   filePrint->setWhatsThis (i18n("Prints out the actual history"));

   fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
   fileQuit->setToolTip(i18n("Quits the application"));
   fileQuit->setWhatsThis (i18n("Quits the application"));

// The "Edit" menu
   editCut = KStdAction::cut(phraseList, SLOT(cut()), actionCollection());
   editCut->setToolTip(i18n("Cuts the selected section and puts it to the clipboard"));
   editCut->setWhatsThis (i18n("Cuts the selected section and puts it to the clipboard. If there is some text selected in the edit field it is placed it on the clipboard. Otherwise the selected sentences in the history (if any) are placed on the clipboard."));

   editCopy = KStdAction::copy(phraseList, SLOT(copy()), actionCollection());
   editCopy->setToolTip(i18n("Copies the selected section to the clipboard"));
   editCopy->setWhatsThis (i18n("Copies the selected section to the clipboard. If there is some text selected in the edit field it is copied to the clipboard. Otherwise the selected sentences in the history (if any) are copied to the clipboard."));

   editPaste = KStdAction::paste(phraseList, SLOT(paste()), actionCollection());
   editPaste->setToolTip(i18n("Pastes the clipboard contents to actual position"));
   editPaste->setWhatsThis (i18n("Pastes the clipboard contents at the current cursor position into the edit field."));

   editSpeak = new KAction (KIcon("speak"), i18n("&Speak"), actionCollection(),"edit_speak");
   connect(editSpeak, SIGNAL(triggered(bool)), phraseList, SLOT(speak()));
   editSpeak->setToolTip(i18n("Speaks the currently active sentence(s)"));
   editSpeak->setWhatsThis (i18n("Speaks the currently active sentence(s). If there is some text in the edit field it is spoken. Otherwise the selected sentences in the history (if any) are spoken."));

// The "Phrase book" menu
   phrasebookEdit = new KAction(i18n("&Edit..."), actionCollection(),"phrasebook_edit");
   connect(phrasebookEdit, SIGNAL(triggered(bool)), this, SLOT(slotEditPhrasebook()));

// The "Options" menu
   viewMenuBar = KStdAction::showMenubar(this, SLOT(slotViewMenuBar()), actionCollection());
   // FIXME: Disable so it will compile.
   // viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
   // viewToolBar->setToolTip(i18n("Enables/disables the toolbar"));
   // viewToolBar->setWhatsThis (i18n("Enables/disables the toolbar"));

   viewPhrasebookBar = new KToggleAction (i18n("Show P&hrasebook Bar"), actionCollection(), "showPhrasebookBar");
   connect(viewPhrasebookBar, SIGNAL(triggered(bool)), this, SLOT(slotViewPhrasebookBar()));
   viewPhrasebookBar->setToolTip(i18n("Enables/disables the phrasebook bar"));
   viewPhrasebookBar->setWhatsThis (i18n("Enables/disables the phrasebook bar"));

   viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
   viewStatusBar->setToolTip(i18n("Enables/disables the statusbar"));
   viewStatusBar->setWhatsThis (i18n("Enables/disables the statusbar"));

   configureTTS = new KAction (KIcon("configure"), i18n("&Configure KMouth..."), actionCollection(), "configureTTS");
   connect(configureTTS, SIGNAL(triggered(bool)), this, SLOT(slotConfigureTTS()));
   configureTTS->setToolTip(i18n("Opens the configuration dialog"));
   configureTTS->setWhatsThis (i18n("Opens the configuration dialog"));

// The "Help" menu
   // The "Help" menu will automatically get created.

// The popup menu of the list of spoken sentences
   phraseListSpeak = new KAction (KIcon("speak"), i18n("&Speak"), actionCollection(), "phraselist_speak");
   phraseListSpeak->setToolTip(i18n("Speaks the currently selected phrases in the history"));
   connect(phraseListSpeak, SIGNAL(triggered(bool)), phraseList, SLOT(speakListSelection()));
   phraseListSpeak->setWhatsThis (i18n("Speaks the currently selected phrases in the history"));

   phraseListRemove = new KAction (KIcon("editdelete"), i18n("&Delete"), actionCollection(), "phraselist_remove");
   connect(phraseListRemove, SIGNAL(triggered(bool)), phraseList, SLOT(removeListSelection()));
   phraseListRemove->setToolTip(i18n("Deletes the currently selected phrases from the history"));
   phraseListRemove->setWhatsThis (i18n("Deletes the currently selected phrases from the history"));

   phraseListCut = new KAction (KIcon("editcut"), i18n("Cu&t"), actionCollection(), "phraselist_cut");
   connect(phraseListCut, SIGNAL(triggered(bool)), phraseList, SLOT(cutListSelection()));
   phraseListCut->setToolTip(i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));
   phraseListCut->setWhatsThis (i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));

   phraseListCopy   = new KAction (KIcon("editcopy"), i18n("&Copy"), actionCollection(), "phraselist_copy");
   connect(phraseListCopy, SIGNAL(triggered(bool)), phraseList, SLOT(copyListSelection()));
   phraseListCut->setToolTip(i18n("Copies the currently selected phrases from the history to the clipboard"));
   phraseListCut->setWhatsThis (i18n("Copies the currently selected phrases from the history to the clipboard"));

   phraselistSelectAll = new KAction (i18n("Select &All Entries"), actionCollection(),"phraselist_select_all");
   connect(phraselistSelectAll, SIGNAL(triggered(bool)), phraseList, SLOT(selectAllEntries()));
   phraselistSelectAll->setToolTip(i18n("Selects all phrases in the history"));
   phraselistSelectAll->setWhatsThis (i18n("Selects all phrases in the history"));

   phraselistDeselectAll = new KAction (i18n("D&eselect All Entries"), actionCollection(),"phraselist_deselect_all");
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
  statusBar()->insertItem(i18n("Ready."), ID_STATUS_MSG);
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
  slotStatusMsg(i18n("Ready."));
}

void KMouthApp::saveOptions() {
   if (isConfigured) {
      config->setGroup("General Options");
      config->writeEntry("Geometry", size());
      config->writeEntry("Show Menubar", viewMenuBar->isChecked());
      // FIXME: Toolbar disabled so it will compile.
      // config->writeEntry("Show Toolbar", viewToolBar->isChecked());
      config->writeEntry("Show Phrasebook Bar", viewPhrasebookBar->isChecked());
      config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
      // FIXME: KToolBar no longer has barPos() method.
      // config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

      if (phraseList != 0)
         phraseList->saveCompletionOptions(config);
      optionsDialog->saveOptions(config);
      toolBar("mainToolBar")->saveSettings (config, "mainToolBar");
      toolBar("phrasebookBar")->saveSettings (config, "phrasebookBar");
   }
}


void KMouthApp::readOptions()
{
  config->setGroup("General Options");

  // bar status settings
  bool bViewMenubar = config->readEntry("Show Menubar", true);
  viewMenuBar->setChecked(bViewMenubar);
  slotViewMenuBar();

  // FIXME: Toolbar disabled so it will compile.
  // bool bViewToolbar = config->readEntry("Show Toolbar", QVariant(true)).toBool();
  // viewToolBar->setChecked(bViewToolbar);
  // slotViewToolBar();

  bool bViewPhrasebookbar = config->readEntry("Show Phrasebook Bar", true);
  viewPhrasebookBar->setChecked(bViewPhrasebookbar);

  bool bViewStatusbar = config->readEntry("Show Statusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();


  // bar position settings
  // FIXME:
  // KToolBar::BarPosition toolBarPos;
  // toolBarPos=(KToolBar::BarPosition) config->readEntry("ToolBarPos", int(KToolBar::Top));
  // toolBar("mainToolBar")->setBarPos(toolBarPos);

  QSize size=config->readEntry("Geometry",QSize());
  if(!size.isEmpty())
  {
    resize(size);
  }

  optionsDialog->readOptions(config);

  toolBar("mainToolBar")->applySettings (config, "mainToolBar");
  toolBar("phrasebookBar")->applySettings (config, "phrasebookBar");

  QString standardBook = KApplication::kApplication()->dirs()->findResource("appdata", "standard.phrasebook");
  if (!standardBook.isEmpty()) {
     PhraseBook book;
     book.open(KUrl::fromPathOrUrl( standardBook ));
     slotPhrasebookConfirmed(book);
  }
  if (phraseList != 0)
     phraseList->readCompletionOptions(config);
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

   slotStatusMsg(i18n("Ready."));
}

void KMouthApp::slotFileSaveAs() {
   slotStatusMsg(i18n("Saving history with a new filename..."));

   phraseList->save();

   slotStatusMsg(i18n("Ready."));
}

void KMouthApp::slotFilePrint()
{
  slotStatusMsg(i18n("Printing..."));

  KPrinter printer;
  if (printer.setup(this))
  {
    phraseList->print(&printer);
  }

  slotStatusMsg(i18n("Ready."));
}

void KMouthApp::slotFileQuit()
{
  slotStatusMsg(i18n("Exiting..."));
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
#warning "kde4: how remove it ?.???"
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

  slotStatusMsg(i18n("Ready."));
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

  slotStatusMsg(i18n("Ready."));
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

  slotStatusMsg(i18n("Ready."));
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

  slotStatusMsg(i18n("Ready."));
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

   QList<KAction *> actions = phrases->actions ();
   for (int actNdx = 0; actNdx < actions.count(); ++actNdx) {
      actions[actNdx]->unplugAll();
   }
   delete phrases;

   phrases = new KActionCollection (actionCollection());
   book.addToGUI (popup, toolbar, phrases, this, SLOT(slotPhraseSelected (const QString &)));

   QString bookLocation = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/");
   if (!bookLocation.isNull() && !bookLocation.isEmpty()) {
      book.save (KUrl::fromPathOrUrl( bookLocation + "standard.phrasebook" ));
   }
}

void KMouthApp::slotConfigurationChanged()
{
   optionsDialog->saveOptions (config);
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
