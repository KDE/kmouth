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

// $Id$

// include files for QT
#include <qdir.h>
#include <qpainter.h>

// include files for KDE
#include <kiconloader.h>
#include <kmenubar.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kconfig.h>
#include <kstdaction.h>
#include <kprinter.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>

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
   config=kapp->config();

   ///////////////////////////////////////////////////////////////////
   // call inits to invoke all other construction parts
   initStatusBar();
   initPhraseList();
   initActions();
   optionsDialog = new OptionsDialog(this);
   connect (optionsDialog, SIGNAL(configurationChanged ()),
            this, SLOT(slotConfigurationChanged ()));

   phrases = new KActionCollection (this);

   readOptions();
   ConfigWizard *wizard = new ConfigWizard (this, "ConfigWizard", config);
   if (wizard->configurationNeeded ()) {
      if (wizard->requestConfiguration ()) {
         saveOptions();
         wizard->saveConfig (config);
         readOptions();
         isConfigured = true;
      }
      else
         isConfigured = false;
   }
   else
      isConfigured = true;
   delete wizard;

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
   fileOpen = new KAction(i18n("&Open as History..."), "phrasehistory_open", KShortcut("Ctrl+O"), this, SLOT(slotFileOpen()), actionCollection(),"file_open");
   fileOpen->setStatusText(i18n("Opens an existing file as history"));
   fileOpen->setWhatsThis (i18n("Opens an existing file as history"));

   fileSaveAs = new KAction(i18n("&Save History As..."), "phrasehistory_save", KShortcut("Ctrl+S"), this, SLOT(slotFileSaveAs()), actionCollection(),"file_save_as");
   fileSaveAs->setStatusText(i18n("Saves the actual history as..."));
   fileSaveAs->setWhatsThis (i18n("Saves the actual history as..."));

   filePrint = new KAction(i18n("&Print History..."), "phrasehistory_print", KShortcut("Ctrl+P"), this, SLOT(slotFilePrint()), actionCollection(),"file_print");
   filePrint->setStatusText(i18n("Prints out the actual history"));
   filePrint->setWhatsThis (i18n("Prints out the actual history"));

   fileQuit = KStdAction::quit(this, SLOT(slotFileQuit()), actionCollection());
   fileQuit->setStatusText(i18n("Quits the application"));
   fileQuit->setWhatsThis (i18n("Quits the application"));

// The "Edit" menu
   editCut = KStdAction::cut(phraseList, SLOT(cut()), actionCollection());
   editCut->setStatusText(i18n("Cuts the selected section and puts it to the clipboard"));
   editCut->setWhatsThis (i18n("Cuts the selected section and puts it to the clipboard. If there is some text selected in the edit field it is placed it on the clipboard. Otherwise the selected sentences in the history (if any) are placed on the clipboard."));

   editCopy = KStdAction::copy(phraseList, SLOT(copy()), actionCollection());
   editCopy->setStatusText(i18n("Copies the selected section to the clipboard"));
   editCopy->setWhatsThis (i18n("Copies the selected section to the clipboard. If there is some text selected in the edit field it is copied to the clipboard. Otherwise the selected sentences in the history (if any) are copied to the clipboard."));

   editPaste = KStdAction::paste(phraseList, SLOT(paste()), actionCollection());
   editPaste->setStatusText(i18n("Pastes the clipboard contents to actual position"));
   editPaste->setWhatsThis (i18n("Pastes the clipboard contents at the current cursor position into the edit field."));

   editSpeak = new KAction (i18n("&Speak"), "speak", 0, phraseList, SLOT(speak()), actionCollection(),"edit_speak");
   editSpeak->setStatusText(i18n("Speaks the currently active sentence(s)"));
   editSpeak->setWhatsThis (i18n("Speaks the currently active sentence(s). If there is some text in the edit field it is spoken. Otherwise the selected sentences in the history (if any) are spoken."));

// The "Phrase book" menu
   phrasebookEdit = new KAction(i18n("&Edit..."), 0, 0, this, SLOT(slotEditPhrasebook()), actionCollection(),"phrasebook_edit");

// The "Options" menu
   viewToolBar = KStdAction::showToolbar(this, SLOT(slotViewToolBar()), actionCollection());
   viewToolBar->setStatusText(i18n("Enables/disables the toolbar"));
   viewToolBar->setWhatsThis (i18n("Enables/disables the toolbar"));

   viewStatusBar = KStdAction::showStatusbar(this, SLOT(slotViewStatusBar()), actionCollection());
   viewStatusBar->setStatusText(i18n("Enables/disables the statusbar"));
   viewStatusBar->setWhatsThis (i18n("Enables/disables the statusbar"));

   configureTTS = new KAction (i18n("&Confige KMouth..."), "configure", 0, this, SLOT(slotConfigureTTS()), actionCollection(), "configureTTS");
   configureTTS->setStatusText(i18n("Opens the configuration dialog"));
   configureTTS->setWhatsThis (i18n("Opens the configuration dialog"));

// The "Help" menu
   // The "Help" menu will automatically get created.

// The popup menu of the list of spoken sentences
   phraseListSpeak = new KAction (i18n("&Speak"),  "speak",  0, phraseList, SLOT(speakListSelection()),  actionCollection(), "phraselist_speak");
   phraseListSpeak->setStatusText(i18n("Speaks the currently selected phrases in the history"));
   phraseListSpeak->setWhatsThis (i18n("Speaks the currently selected phrases in the history"));

   phraseListRemove = new KAction (i18n("&Remove"), "remove", 0, phraseList, SLOT(removeListSelection()), actionCollection(), "phraselist_remove");
   phraseListRemove->setStatusText(i18n("Removes the currently selected phrases from the history"));
   phraseListRemove->setWhatsThis (i18n("Removes the currently selected phrases from the history"));

   phraseListCut = new KAction (i18n("&Cut"),   "editcut", 0, phraseList, SLOT(cutListSelection()),    actionCollection(), "phraselist_cut");
   phraseListCut->setStatusText(i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));
   phraseListCut->setWhatsThis (i18n("Cuts the currently selected phrases from the history and puts them to the clipboard"));

   phraseListCopy   = new KAction (i18n("&Copy"), "editcopy", 0, phraseList, SLOT(copyListSelection()),   actionCollection(), "phraselist_copy");
   phraseListCut->setStatusText(i18n("Copies the currently selected phrases from the history to the clipboard"));
   phraseListCut->setWhatsThis (i18n("Copies the currently selected phrases from the history to the clipboard"));

   phraselistSelectAll = new KAction (i18n("&Select All Entries"), 0, 0, phraseList, SLOT(selectAllEntries()), actionCollection(),"phraselist_select_all");
   phraselistSelectAll->setStatusText(i18n("Selects all phrases in the history"));
   phraselistSelectAll->setWhatsThis (i18n("Selects all phrases in the history"));

   phraselistDeselectAll = new KAction (i18n("&Deselect All Entries"), 0, 0, phraseList, SLOT(deselectAllEntries()), actionCollection(),"phraselist_deselect_all");
   phraselistDeselectAll->setStatusText(i18n("Deselects all phrases in the history"));
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

void KMouthApp::openDocumentFile(const KURL& url)
{
  slotStatusMsg(i18n("Opening file..."));

  phraseList->open (url);
  slotStatusMsg(i18n("Ready."));
}

void KMouthApp::saveOptions() {
   if (isConfigured) {
      config->setGroup("General Options");
      config->writeEntry("Geometry", size());
      config->writeEntry("Show Toolbar", viewToolBar->isChecked());
      config->writeEntry("Show Statusbar",viewStatusBar->isChecked());
      config->writeEntry("ToolBarPos", (int) toolBar("mainToolBar")->barPos());

      optionsDialog->saveOptions(config);
      toolBar("mainToolBar")->saveSettings (config, "mainToolBar");
      toolBar("phrasebookBar")->saveSettings (config, "phrasebookBar");
   }
}


void KMouthApp::readOptions()
{
  config->setGroup("General Options");

  // bar status settings
  bool bViewToolbar = config->readBoolEntry("Show Toolbar", true);
  viewToolBar->setChecked(bViewToolbar);
  slotViewToolBar();

  bool bViewStatusbar = config->readBoolEntry("Show Statusbar", true);
  viewStatusBar->setChecked(bViewStatusbar);
  slotViewStatusBar();


  // bar position settings
  KToolBar::BarPosition toolBarPos;
  toolBarPos=(KToolBar::BarPosition) config->readNumEntry("ToolBarPos", KToolBar::Top);
  toolBar("mainToolBar")->setBarPos(toolBarPos);

  QSize size=config->readSizeEntry("Geometry");
  if(!size.isEmpty())
  {
    resize(size);
  }

  optionsDialog->readOptions(config);

  toolBar("mainToolBar")->applySettings (config, "mainToolBar");
  toolBar("phrasebookBar")->applySettings (config, "phrasebookBar");

  QString standardBook = KApplication::kApplication()->dirs()->findResource("appdata", "standard.phrasebook");
  if (!standardBook.isNull() && !standardBook.isEmpty()) {
     PhraseBook book;
     book.open(standardBook);
     slotPhrasebookConfirmed(book);
  }
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
  if (memberList)
  {
    for(w=memberList->first(); w!=0; w=memberList->first())
    {
      // only close the window if the closeEvent is accepted. If the user presses Cancel on the saveModified() dialog,
      // the window and the application stay open.
      if(!w->close())
         break;
      memberList->removeRef(w);
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
   optionsDialog->show();
}


void KMouthApp::slotStatusMsg(const QString &text)
{
  ///////////////////////////////////////////////////////////////////
  // change status message permanently
  statusBar()->clear();
  statusBar()->changeItem(text, ID_STATUS_MSG);
}

void KMouthApp::slotPhrasebookConfirmed (PhraseBook &book) {
   QString name = "phrasebooks";
   QPopupMenu *popup = (QPopupMenu *)factory()->container(name, this);
   KToolBar *toolbar = toolBar ("phrasebookBar");

   KActionPtrList actions = phrases->actions ();
   KActionPtrList::iterator iter;
   for (iter = actions.begin(); iter != actions.end(); ++iter) {
      (*iter)->unplugAll();
   }
   delete phrases;

   phrases = new KActionCollection (this, actionCollection());
   book.addToGUI (popup, toolbar, phrases, this, SLOT(slotPhraseSelected (const QString &)));

   QString bookLocation = KApplication::kApplication()->dirs()->saveLocation ("appdata", "/");
   if (!bookLocation.isNull() && !bookLocation.isEmpty()) {
      book.save (bookLocation + "standard.phrasebook");
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

/*
 * $Log$
 * Revision 1.3  2003/01/18 15:47:46  gunnar
 * Two small changes
 *
 * Revision 1.2  2003/01/18 07:29:11  binner
 * CVS_SILENT i18n style guide fixes
 *
 * Revision 1.1  2003/01/17 23:09:36  gunnar
 * Imported KMouth into kdeaccessibility
 *
 * Revision 1.39  2003/01/17 16:03:00  gunnar
 * Help buttons added and small bug when aborting the wizard fixed
 *
 * Revision 1.38  2003/01/12 11:37:05  gunnar
 * Improved format list of file selectors / several small changes
 *
 * Revision 1.37  2002/12/30 12:08:07  gunnar
 * Configuration wizard improved
 *
 * Revision 1.36  2002/12/06 08:54:16  gunnar
 * Changed the menu entry for configuring KMouth
 *
 * Revision 1.35  2002/12/04 16:22:02  gunnar
 * Include *.moc files
 *
 * Revision 1.34  2002/11/25 16:24:53  gunnar
 * Changes on the way to version 0.7.99.1rc1
 *
 * Revision 1.33  2002/11/22 08:48:34  gunnar
 * Implemented functionality that belongs to the new options in the options dialog
 *
 * Revision 1.32  2002/11/21 21:33:26  gunnar
 * Extended parameter dialog and added wizard for the first start
 *
 * Revision 1.31  2002/11/20 10:55:43  gunnar
 * Improved the keyboard accessibility
 *
 * Revision 1.30  2002/11/19 17:48:14  gunnar
 * Prevented both the parallel start of multiple KMouth instances and the parallel opening of multiple Phrase book edit windows
 *
 * Revision 1.29  2002/11/11 21:25:42  gunnar
 * Moved the parts concerning phrase books into a static library
 *
 * Revision 1.28  2002/11/08 09:01:46  gunnar
 * Improved menu structures for importing standard phrase books
 *
 * Revision 1.27  2002/11/06 19:15:07  gunnar
 * import of standard phrase books added
 *
 * Revision 1.26  2002/11/06 17:28:02  gunnar
 * Code for preventing double keyboard shortcuts extended for imported/dragged/pasted phrasebooks
 *
 * Revision 1.25  2002/10/29 18:11:17  gunnar
 * Implemented opening and saving of a standard phrasebook
 *
 * Revision 1.24  2002/10/29 16:16:05  gunnar
 * Connection from the phrase book to the phrase edit field added
 *
 * Revision 1.23  2002/10/23 22:19:29  gunnar
 * Cut, copy and paste features of the phrase book edit dialog improved
 *
 * Revision 1.22  2002/10/23 17:42:52  gunnar
 * Icons added to the items of the phrase book edit dialog
 *
 * Revision 1.21  2002/10/22 16:13:23  gunnar
 * Popup menu in the phrase book dialog added
 *
 * Revision 1.20  2002/10/21 18:30:50  gunnar
 * First version of the phrase book edit dialog added
 *
 * Revision 1.19  2002/10/21 16:13:30  gunnar
 * phrase book format implemented
 *
 * Revision 1.18  2002/10/17 17:05:46  gunnar
 * Small changes
 *
 * Revision 1.17  2002/10/15 15:53:26  gunnar
 * Improved help texts
 *
 * Revision 1.16  2002/10/07 17:09:33  gunnar
 * What's this? texts added
 *
 * Revision 1.15  2002/10/04 09:14:50  gunnar
 * some small changes and english handbook added
 *
 * Revision 1.14  2002/10/02 16:55:17  gunnar
 * Removed some compiler warnings and implementing the feature of opening a history at the start of KMouth
 *
 * Revision 1.13  2002/10/02 14:55:32  gunnar
 * Fixed Speak-empty-phrase-crash bug and added some i18n() encodings
 *
 * Revision 1.12  2002/09/26 17:10:46  gunnar
 * Several small changes
 *
 * Revision 1.11  2002/09/18 09:30:40  gunnar
 * A nuber of small changes
 *
 * Revision 1.10  2002/09/13 11:51:24  gunnar
 * Added printing support
 *
 * Revision 1.9  2002/09/13 09:40:37  gunnar
 * Added support for opening a file as history
 *
 * Revision 1.8  2002/09/12 15:28:09  gunnar
 * Loading (into the edit line) and saving (the phrase list) implemented
 *
 * Revision 1.7  2002/09/11 16:57:35  gunnar
 * added context menu, and moved the contents of KMouthView and KMouthDoc into a new class PhraseList
 *
 * Revision 1.6  2002/09/08 17:12:55  gunnar
 * Configuration dialog added
 *
 * Revision 1.5  2002/09/01 08:32:54  gunnar
 * toolbar icon and application icon added
 *
 * Revision 1.4  2002/08/29 17:44:17  gunnar
 * Disabled actions that are definitely not needed in the current state of KMouth
 *
 * Revision 1.3  2002/08/29 09:14:12  gunnar
 * Added functionality for scrolling through the list of spoken sentences
 *
 * Revision 1.2  2002/08/28 19:20:04  gunnar
 * Added an edit field and a list of entered lines
 *
 * Revision 1.1.1.1  2002/08/26 14:09:49  gunnar
 * New project started
 *
 */
