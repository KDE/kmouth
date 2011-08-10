/*
 * klanguagebutton.cpp - Adds some methods for inserting languages.
 *
 * Copyright (c) 1999-2000 Hans Petter Bieker <bieker@kde.org>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.trolltech.com/
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#define INCLUDE_MENUITEM_DEF
#include <tqpopupmenu.h>
#include <kstandarddirs.h>

#include "klanguagebutton.h"

#include <kdebug.h>


static inline void checkInsertPos( TQPopupMenu *popup, const TQString & str,
                                   int &index )
{
  if ( index == -2 )
    index = popup->count();
  if ( index != -1 )
    return;

  int a = 0;
  int b = popup->count();
  while ( a <= b )
  {
    int w = ( a + b ) / 2;

    int id = popup->idAt( w );
    int j = str.localeAwareCompare( popup->text( id ) );

    if ( j > 0 )
      a = w + 1;
    else
      b = w - 1;
  }

  index = a; // it doesn't really matter ... a == b here.
}

static inline TQPopupMenu * checkInsertIndex( TQPopupMenu *popup,
                            const TQStringList *tags, const TQString &submenu )
{
  int pos = tags->findIndex( submenu );

  TQPopupMenu *pi = 0;
  if ( pos != -1 )
  {
    TQMenuItem *p = popup->findItem( pos );
    pi = p ? p->popup() : 0;
  }
  if ( !pi )
    pi = popup;

  return pi;
}


KLanguageButton::~KLanguageButton()
{
  delete m_tags;
}

KLanguageButton::KLanguageButton( TQWidget * parent, const char *name )
: TQPushButton( parent, name ),
	m_popup( 0 ),
	m_oldPopup( 0 )
{
  m_tags = new TQStringList;

  clear();
}

void KLanguageButton::insertItem( const TQIconSet& icon, const TQString &text,
                      const TQString &tag, const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( icon, text, count(), index );
  m_tags->append( tag );
}

void KLanguageButton::insertItem( const TQString &text, const TQString &tag,
                                  const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  checkInsertPos( pi, text, index );
  pi->insertItem( text, count(), index );
  m_tags->append( tag );
}

void KLanguageButton::insertSeparator( const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  pi->insertSeparator( index );
  m_tags->append( TQString() );
}

void KLanguageButton::insertSubmenu( const TQString &text, const TQString &tag,
                                     const TQString &submenu, int index )
{
  TQPopupMenu *pi = checkInsertIndex( m_popup, m_tags, submenu );
  TQPopupMenu *p = new TQPopupMenu( pi );
  checkInsertPos( pi, text, index );
  pi->insertItem( text, p, count(), index );
  m_tags->append( tag );
  connect( p, TQT_SIGNAL( activated( int ) ),
                        TQT_SLOT( slotActivated( int ) ) );
  connect( p, TQT_SIGNAL( highlighted( int ) ), this,
                        TQT_SIGNAL( highlighted( int ) ) );
}

void KLanguageButton::insertLanguage( const TQString& path, const TQString& name,
                        const TQString& sub, const TQString &submenu, int index )
{
  TQString output = name + TQString::tqfromLatin1( " (" ) + path +
                   TQString::tqfromLatin1( ")" );
#if 0
  // Nooooo ! Country != language
  TQPixmap flag( locate( "locale", sub + path +
                TQString::tqfromLatin1( "/flag.png" ) ) );
#endif
  insertItem( output, path, submenu, index );
}

void KLanguageButton::slotActivated( int index )
{
  // Update caption and iconset:
  if ( m_current == index )
    return;

  setCurrentItem( index );

  // Forward event from popup menu as if it was emitted from this widget:
  emit activated( index );
}

int KLanguageButton::count() const
{
  return m_tags->count();
}

void KLanguageButton::clear()
{
  m_tags->clear();

  delete m_oldPopup;
  m_oldPopup = m_popup;
  m_popup = new TQPopupMenu( this );

  setPopup( m_popup );

  connect( m_popup, TQT_SIGNAL( activated( int ) ),
                        TQT_SLOT( slotActivated( int ) ) );
  connect( m_popup, TQT_SIGNAL( highlighted( int ) ),
                        TQT_SIGNAL( highlighted( int ) ) );

  setText( TQString() );
  setIconSet( TQIconSet() );
}

/*void KLanguageButton::changeLanguage( const TQString& name, int i )
{
  if ( i < 0 || i >= count() )
    return;
  TQString output = name + TQString::tqfromLatin1( " (" ) + tag( i ) +
                   TQString::tqfromLatin1( ")" );
  changeItem( output, i );
}*/

bool KLanguageButton::containsTag( const TQString &str ) const
{
  return m_tags->contains( str ) > 0;
}

TQString KLanguageButton::currentTag() const
{
  return *m_tags->at( currentItem() );
}

TQString KLanguageButton::tag( int i ) const
{
  if ( i < 0 || i >= count() )
  {
    kdDebug() << "KLanguageButton::tag(), unknown tag " << i << endl;
    return TQString();
  }
  return *m_tags->at( i );
}

int KLanguageButton::currentItem() const
{
  return m_current;
}

void KLanguageButton::setCurrentItem( int i )
{
  if ( i < 0 || i >= count() )
    return;
  m_current = i;

  setText( m_popup->text( m_current ) );
  TQIconSet *icon = m_popup->iconSet( m_current );
  if( icon )
    setIconSet( *icon );
  else
    setIconSet( TQPixmap() );
}

void KLanguageButton::setCurrentItem( const TQString &code )
{
  int i = m_tags->findIndex( code );
  if ( code.isNull() )
    i = 0;
  if ( i != -1 )
    setCurrentItem( i );
}

#include "klanguagebutton.moc"
