/***************************************************************************
                          kmouth.h  -  description
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


#ifndef KMOUTH_H
#define KMOUTH_H


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt

// include files for KDE
#include <kapplication.h>
#include <kmainwindow.h>
#include <kaccel.h>
#include <kaction.h>
#include <kurl.h>

// forward declaration of the KMouth classes
class PhraseList;
class OptionsDialog;
class TextToSpeechSystem;
class PhraseBookDialog;
class PhraseBook;

/**
  * The base class for KMouth application windows. It sets up the main
  * window and reads the config file as well as providing a menubar, toolbar
  * and statusbar.
  * KMouthApp reimplements the methods that KMainWindow provides for main window handling and supports
  * full session management as well as using KActions.
  * @see KMainWindow
  * @see KApplication
  * @see KConfig
  *
  * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
  * @version KDevelop version 1.2 code generation
  */
class KMouthApp : public KMainWindow
{
  Q_OBJECT

  public:
    /** construtor of KMouthApp, calls all init functions to create the application.
     */
    KMouthApp(TQWidget* parent=0, const char* name=0);
    ~KMouthApp();
    /** Returns true if the configuration wizard was not needed or when it
     * was successfully completed.
     */
    bool configured();
    /** opens a file specified by commandline option
     */
    void openDocumentFile(const KURL& url=KURL());

    TextToSpeechSystem *getTTSSystem() const;

    /** called by PhraseList in order to enable or disable the actions depending
     * on the contents of the phrase list.
     */
    void enableMenuEntries(bool existSelectedEntries, bool existDeselectedEntries);

  protected:
    /** save general Options like all bar positions and status as well as the geometry and the recent file list to the configuration
     * file
     */
    void saveOptions();
    /** read general Options again and initialize all variables like the recent file list
     */
    void readOptions();
    /** initializes the phrase list */
    void initPhraseList();
    /** initializes the KActions of the application */
    void initActions();
    /** sets up the statusbar for the main window by initialzing a statuslabel.
     */
    void initStatusBar();
    /** queryClose is called by KTMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * @see KTMainWindow#queryClose
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryClose();
    /** queryExit is called by KTMainWindow when the last window of the application is going to be closed during the closeEvent().
     * Against the default implementation that just returns true, this calls saveOptions() to save the settings of the last window's
     * properties.
     * @see KTMainWindow#queryExit
     * @see KTMainWindow#closeEvent
     */
    virtual bool queryExit();

  public slots:
    /** open a file and load it into the history */
    void slotFileOpen();
    /** save a document */
    void slotFileSaveAs();
    /** print the actual file */
    void slotFilePrint();
    /** closes all open windows by calling close() on each memberList item until the list is empty, then quits the application.
     * If queryClose() returns false because the user canceled the saveModified() dialog, the closing breaks.
     */
    void slotFileQuit();
    /** edits the phrase books
     */
    void slotEditPhrasebook ();
    /** toggles the menu bar
     */
    void slotViewMenuBar();
    /** toggles the toolbar
     */
    void slotViewToolBar();
    /** toggles the phrasebook bar
     */
    void slotViewPhrasebookBar();
    /** toggles the statusbar
     */
    void slotViewStatusBar();
    /** configures the TTS system
     */
    void slotConfigureTTS();
    /** changes the statusbar contents for the standard label permanently, used to indicate current actions.
     * @param text the text that is displayed in the statusbar
     */
    void slotStatusMsg(const TQString &text);

    void slotPhrasebookConfirmed (PhraseBook &book);

    void slotConfigurationChanged();

    void slotPhraseSelected (const TQString &phrase);

  private:
    bool isConfigured;
    /** the configuration object of the application */
    KConfig *config;
    /** The phrase list */
    PhraseList *phraseList;
    /** The configuration dialog */
    OptionsDialog *optionsDialog;

    // KAction pointers to enable/disable actions
    KAction* fileOpen;
    KAction* fileSaveAs;
    KAction* filePrint;
    KAction* fileQuit;

    KAction* editCut;
    KAction* editCopy;
    KAction* editPaste;
    KAction* editSpeak;

    KActionCollection *phrases;
    KAction* phrasebookEdit;

    KToggleAction* viewMenuBar;
    KToggleAction* viewToolBar;
    KToggleAction* viewPhrasebookBar;
    KToggleAction* viewStatusBar;
    KAction*       configureTTS;

    KAction* phraseListSpeak;
    KAction* phraseListRemove;
    KAction* phraseListCut;
    KAction* phraseListCopy;
    KAction* phraselistSelectAll;
    KAction* phraselistDeselectAll;
};

#endif // KMOUTH_H
