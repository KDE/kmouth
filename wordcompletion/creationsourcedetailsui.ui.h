/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#ifndef CREATIONSOURCEDETAILSUI_UI_H
#define CREATIONSOURCEDETAILSUI_UI_H

void CreationSourceDetailsUI::init() {
    languageButton = new KLanguageButton (this);
    languageLabel->setBuddy (languageButton);
    languageButton->setWhatsThis( i18n("With this combo box you decide which language should be associated with the new dictionary."));
    
    languageButton->showLanguageCodes(true);
    languageButton->loadAllLanguages();
    languageButton->insertLanguage("??", i18n("Other"));
    
   connect (languageButton, SIGNAL(activated(int)), this, SLOT(languageButton_activated(int)));
}

void CreationSourceDetailsUI::languageButton_activated (int) {
   if (languageButton->current() == "??") {
     QString customLanguage = KInputDialog::getText(i18n("Create Custom Language"), i18n("Please enter the code for the custom language:"));
     
     if (languageButton->contains(customLanguage)) {
        languageButton->setCurrentItem(customLanguage);
     }
     else {
        languageButton->insertLanguage(customLanguage, i18n("without name"));
        languageButton->setCurrentItem(customLanguage);
     }
   }
}

#endif // CREATIONSOURCEDETAILSUI_UI_H
