/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


void CreationSourceUI::init() {
   group1 = new QButtonGroup (0);
   group1->insert (createButton);
   group1->insert (mergeButton);
   group1->insert (emptyButton);
   group2 = new QButtonGroup (0);
   group2->insert (kdeDocButton);
   group2->insert (fileButton);
   group2->insert (directoryButton);
}


void CreationSourceUI::destroy() {
   delete group1;
   delete group2;
}
