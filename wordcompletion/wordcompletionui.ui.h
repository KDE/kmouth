/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void WordCompletionUI::init() {
    languageButton = new KLanguageButton (selectedDictionaryDetails, "languageButton");
    selectedDictionaryDetailsLayout->addWidget (languageButton, 1, 1);
    languageLabel->setBuddy (languageButton);
    
    loadLanguageList(languageButton);
}
