/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014-2015  by Luis Lucas      (luisfrlucas@gmail.com)
 *                                Joao Carreira   (jfmcarreira@gmail.com)
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
 * \file     SubWindowHandle.h
 * \brief    Sub windows handling
 */

#ifndef __SUBWINDOWHANDLE_H__
#define __SUBWINDOWHANDLE_H__

#include "config.h"
#include "PlaYUVerAppDefs.h"
#if( QT_VERSION_PLAYUVER == 5 )
#include <QtWidgets>
#elif( QT_VERSION_PLAYUVER == 4 )
#include <QtGui>
#endif

namespace plaYUVer
{

class PlaYUVerMdiSubWindow;

class SubWindowHandle: public QScrollArea
{
Q_OBJECT

private:
  QPoint m_cLastScroll;
  QPoint m_cCurrScroll;

  UInt m_uiCategory;
  QString m_cWindowName;

  PlaYUVerMdiSubWindow* m_cSubWindow;

public:
  enum SubWindowCategories
  {
    SUBWINDOW = 0,
    VIDEO_SUBWINDOW = 1,
    VIDEO_STREAM_SUBWINDOW = 2,
    MODULE_SUBWINDOW = 4,
    PLOT_SUBWINDOW = 8,
  };

  SubWindowHandle( QWidget *, UInt );
  ~SubWindowHandle();

  /**
   * Show the image at its original size
   */
  virtual Void normalSize() = 0;
  /**
   * Scale the image (zoomed in or out) to fit on the window.
   */
  virtual Void zoomToFit() = 0;
  /**
   * Scale the image (zoomed in or out) to speficied absolute zoom value (1.0 = original size).
   */
  virtual Void zoomToFactor( Double factor, QPoint center = QPoint() ) = 0;
  /**
   * Scale the image by a given factor
   * @param factor factor of scale. Ex: 1.2 scale the image up by 20% and
   *        0.8 scale the image down by 25%
   */
  virtual Void scaleView( Double, QPoint center = QPoint() ) = 0;

  virtual Double getScaleFactor() = 0;

  /**
   * Size related functions
   */
  //virtual QSize sizeHint() const;
  virtual QSize sizeHint( const QSize & ) const;

  /**
   * Window name
   */

  Void setWindowName( QString );
  QString getWindowName();

  virtual Bool mayClose();

  UInt getCategory()
  {
    return m_uiCategory;
  }

  Void setSubWindow(PlaYUVerMdiSubWindow* subWindow )
  {
    m_cSubWindow = subWindow;
  }

  Void closeSubWindow();

protected:
  void focusInEvent( QFocusEvent * event );
  void closeEvent( QCloseEvent *event );
  QSize getScrollSize();

Q_SIGNALS:
  /**
   * Notify that zoom factor was changed by internal event (e.g. by mouse wheel)
   */
  void zoomFactorChanged_SWindow( const double, const QPoint );
  /**
   * Notify that scrollbar position was changed by internal event (e.g. by mouse panning )
   */
  void scrollBarMoved_SWindow( const QPoint );
  /**
   * Update stauts bars
   */
  void updateStatusBar( const QString& );

  void aboutToActivate( SubWindowHandle* );
  void aboutToClose( SubWindowHandle* );

public Q_SLOTS:
  void onDestroyed();
  void adjustScrollBarByScale( double scale, QPoint center );
  void adjustScrollBarByOffset( QPoint Offset );
  void updateCurScrollValues();
  void setCurScrollValues();

};

}  // NAMESPACE

#endif // __SUBWINDOWHANDLE_H__

