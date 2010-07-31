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
    TQWhatsThis::add (languageButton, i18n("With this combo box you select the language associated with the selected dictionary."));
    
    loadLanguageList(languageButton);
    languageButton->insertLanguage("??", i18n("Other"), TQString::fromLatin1("l10n/"), TQString::null);
    
   connect (languageButton, TQT_SIGNAL(activated(int)), this, TQT_SLOT(languageButton_activated(int)));
}

void WordCompletionUI::languageButton_activated (int) {
   if (languageButton->currentTag() == "??") {
     TQString customLanguage = KInputDialog::getText(i18n("Create Custom Language"), i18n("Please enter the code for the custom language:"));
     
     if (languageButton->containsTag(customLanguage)) {
        languageButton->setCurrentItem(customLanguage);
     }
     else {
        languageButton->insertLanguage(customLanguage, i18n("without name"), TQString::fromLatin1("l10n/"), TQString::null);
        languageButton->setCurrentItem(customLanguage);
     }
   }
}
