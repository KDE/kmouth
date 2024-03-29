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

#ifndef KMOUTH_H
#define KMOUTH_H

// include files for Qt
#include <QAction>
#include <QPrinter>

// include files for KDE
#include <KSharedConfig>
#include <QUrl>
#include <kxmlguiwindow.h>

// forward declaration of the KMouth classes
class PhraseList;
class OptionsDialog;
class TextToSpeechSystem;

class QLabel;

class KToggleAction;
class KActionCollection;

/**
 * The base class for KMouth application windows. It sets up the main
 * window and reads the config file as well as providing a menubar, toolbar
 * and statusbar.
 * KMouthApp reimplements the methods that KXmlGuiWindow provides for main window handling and supports
 * full session management as well as using KActions.
 * @see KXmlGuiWindow
 *
 * @author Source Framework Automatically Generated by KDevelop, (c) The KDevelop Team.
 * @version KDevelop version 1.2 code generation
 */
class KMouthApp : public KXmlGuiWindow
{
    Q_OBJECT

public:
    /** construtor of KMouthApp, calls all init functions to create the application.
     */
    explicit KMouthApp(QWidget *parent = nullptr, const QString &name = QString());
    ~KMouthApp() override;
    /** Returns true if the configuration wizard was not needed or when it
     * was successfully completed.
     */
    bool configured();
    /** opens a file specified by commandline option
     */
    void openDocumentFile(const QUrl &url = QUrl());

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
    /** queryClose is called by KMainWindow on each closeEvent of a window. Against the
     * default implementation (only returns true), this calles saveModified() on the document object to ask if the document shall
     * be saved if Modified; on cancel the closeEvent is rejected.
     * @see KMainWindow#queryClose
     * @see KMainWindow#closeEvent
     */
    bool queryClose() override;

public Q_SLOTS:
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
    void slotEditPhrasebook();
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
    void slotStatusMsg(const QString &text);

    void slotPhrasebookConfirmed();

    void slotConfigurationChanged();

    void slotPhraseSelected(const QString &phrase);

private:
    bool isConfigured;
    /** The phrase list */
    PhraseList *phraseList;
    /** The configuration dialog */
    OptionsDialog *optionsDialog;

    // QAction pointers to enable/disable actions
    QAction *fileOpen;
    QAction *fileSaveAs;
    QAction *filePrint;
    QAction *fileQuit;

    QAction *editCut;
    QAction *editCopy;
    QAction *editPaste;
    QAction *editSpeak;

    KActionCollection *phrases;
    QAction *phrasebookEdit;

    KToggleAction *viewMenuBar;
    KToggleAction *viewToolBar;
    KToggleAction *viewPhrasebookBar;
    KToggleAction *viewStatusBar;
    QAction *configureTTS;

    QAction *phraseListSpeak;
    QAction *phraseListRemove;
    QAction *phraseListCut;
    QAction *phraseListCopy;
    QAction *phraselistSelectAll;
    QAction *phraselistDeselectAll;

    QLabel *m_statusLabel;

    // Keep QPrinter so settings persist
    QPrinter *printer;
};

#endif // KMOUTH_H
