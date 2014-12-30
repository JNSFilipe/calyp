/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014  by Luis Lucas      (luisfrlucas@gmail.com)
 *                           Joao Carreira   (jfmcarreira@gmail.com)
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/**
 * \file     SubWindowHandle.cpp
 * \brief    Sub windows handling
 */

#include "SubWindowHandle.h"

namespace plaYUVer
{

SubWindowHandle::SubWindowHandle( QWidget * parent, UInt category ) :
        QMdiSubWindow( parent )
{
  m_uiCategory = category;
  m_cWindowName = QString( " " );
}

SubWindowHandle::~SubWindowHandle()
{
}

Void SubWindowHandle::setWindowName( QString name )
{
  m_cWindowName = name;
  setWindowTitle( m_cWindowName );
}

QString SubWindowHandle::getWindowName()
{
  return m_cWindowName;
}

Bool SubWindowHandle::mayClose()
{
  return true;
}

}  // NAMESPACE
