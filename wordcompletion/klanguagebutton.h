/*
 * klangbutton.h - Button with language selection drop down menu.
 *                 Derived from the KLangCombo class by Hans Petter Bieker.
 *
 * Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
 *           (c) 2001      Martijn Klingens   <mklingens@yahoo.com>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#ifndef __KLANGBUTTON_H__
#define __KLANGBUTTON_H__

#include "kpushbutton.h"

/*
 * Extended TQPushButton that shows a menu with submenu for language selection.
 * Essentially just a combo box with a 2-D dataset, but using a real
 * TQComboBox will produce ugly results.
 *
 * Combined version of KTagCombo and KLanguageCombo but using a TQPushButton
 * instead.
 */
class KLanguageButton : public TQPushButton
{
  Q_OBJECT
  TQ_OBJECT

public:
  KLanguageButton(TQWidget *tqparent=0, const char *name=0);
  ~KLanguageButton();

  void insertItem( const TQIconSet& icon, const TQString &text,
                   const TQString &tag, const TQString &submenu = TQString(),
                   int index = -1 );
  void insertItem( const TQString &text, const TQString &tag,
                   const TQString &submenu = TQString(), int index = -1 );
  void insertSeparator( const TQString &submenu = TQString(),
                        int index = -1 );
  void insertSubmenu( const TQString &text, const TQString &tag,
                      const TQString &submenu = TQString(), int index = -1);

  int count() const;
  void clear();
  
  void insertLanguage( const TQString& path, const TQString& name,
                       const TQString& sub = TQString(),
                       const TQString &submenu = TQString(), int index = -1);
  
  /*
   * Tag of the selected item
   */
  TQString currentTag() const;
  TQString tag( int i ) const;
  bool containsTag( const TQString &str ) const;

  /*
   * Set the current item
   */
  int currentItem() const;
  void setCurrentItem( int i );
  void setCurrentItem( const TQString &code );

signals:
  void activated( int index );
  void highlighted( int index );

private slots:
  void slotActivated( int );

private:
  // work space for the new class
  TQStringList *m_tags;  
  TQPopupMenu  *m_popup, *m_oldPopup;
  int         m_current;
};

#endif
