/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void CreationSourceDetailsUI::init() {
    languageButton = new KLanguageButton (this, "languageButton");
    CreationSourceDetailsUILayout->addWidget (languageButton, 2, 1);
    languageLabel->setBuddy (languageButton);
    QWhatsThis::add (languageButton, i18n("With this combo box you decide which language should be associated with the new dictionary."));
    
    loadLanguageList(languageButton);
}



