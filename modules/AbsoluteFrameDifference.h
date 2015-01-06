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
 * \file     AbsoluteFrameDifference.h
 * \brief    Absolute Frame Difference module
 */

#ifndef __ABSOLUTEFRAMEDIFFERENCE_H__
#define __ABSOLUTEFRAMEDIFFERENCE_H__

#include "PlaYUVerModuleIf.h"

#include <cassert>

namespace plaYUVer
{

class AbsoluteFrameDifference: public PlaYUVerModuleIf
{
private:
  PlaYUVerFrame* m_pcFrameDifference;

public:
  AbsoluteFrameDifference();
  virtual ~AbsoluteFrameDifference()
  {
  }
  Void create( PlaYUVerFrame* Input );
  PlaYUVerFrame* process( PlaYUVerFrame* Input1, PlaYUVerFrame* Input2 );
  Void destroy();

};

}  // NAMESPACE

#endif // __FRAMEDIFFERENCE_H__
