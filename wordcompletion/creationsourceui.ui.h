/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#ifndef CREATIONSOURCEUI_UI_H
#define CREATIONSOURCEUI_UI_H

void CreationSourceUI::init() {
   group1 = new Q3ButtonGroup (0);
   group1->insert (createButton);
   group1->insert (mergeButton);
   group1->insert (emptyButton);
   group2 = new Q3ButtonGroup (0);
   group2->insert (kdeDocButton);
   group2->insert (fileButton);
   group2->insert (directoryButton);
}


void CreationSourceUI::destroy() {
   delete group1;
   delete group2;
}

#endif // CREATIONSOURCEUI_UI_H
