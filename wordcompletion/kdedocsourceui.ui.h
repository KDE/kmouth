/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void KDEDocSourceUI::init() {
    languageButton = new KLanguageButton (this);
    languageLabel->setBuddy (languageButton);
    languageButton->setWhatsThis( i18n("With this combo box you select which of the installed languages is used for creating the new dictionary. KMouth will only parse documentation files of this language."));
    
    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();
    
    ooDictURL->setFilter ("*.dic");
}
