/*    This file is a part of plaYUVer project
 *    Copyright (C) 2014  by plaYUVer developers
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
 * \file     imageinterface.cpp
 * \brief    Sub windows handling
 */

#include "ConfigureFormatDialog.h"
#include "SubWindowHandle.h"
#include "ViewArea.h"

namespace plaYUVer
{

SubWindowHandle::SubWindowHandle( QWidget * parent ) :
    QMdiSubWindow( parent )
{
  setParent( parent );

  setAttribute( Qt::WA_DeleteOnClose );
  setBackgroundRole( QPalette::Light );

  // Create a new scroll area inside the sub-window
  m_cScrollArea = new QScrollArea( this );
  setWidget( m_cScrollArea );

  // Create a new interface to show images
  m_cViewArea = new ViewArea( this );
  connect(m_cViewArea, SIGNAL(zoomFactorChanged(double)), this, SLOT(adjustScrollBar(double)));

  // Define the cViewArea as the widget inside the scroll area
  m_cScrollArea->setWidget( m_cViewArea );
  m_cScrollArea->setWidgetResizable(true);

  m_pCurrStream = new InputStream;

  m_pCurrFrameQImage = NULL;
  m_pcCurrentModule = NULL;

  m_cWindowName = QString( " " );
  m_bIsPlaying = true;
}

SubWindowHandle::~SubWindowHandle()
{
  delete m_cViewArea;
  delete m_cScrollArea;
  delete m_pCurrStream;
  if( m_pCurrFrameQImage )
    delete m_pCurrFrameQImage;
  if( m_pcCurrentModule )
  {
    m_pcCurrentModule->m_pcAction->setChecked( false );
    m_pcCurrentModule->destroy();
    m_pcCurrentModule = NULL;
  }
}

Bool SubWindowHandle::loadFile( const QString &fileName )
{
  ConfigureFormatDialog formatDialog( this );

  if( m_pCurrStream->needFormatDialog( fileName ) )
  {
    if( formatDialog.exec() == QDialog::Rejected )
    {
      return false;
    }
    m_pCurrStream->init( fileName, formatDialog.getResolution().width(), formatDialog.getResolution().height(), formatDialog.getPixelFormat(),
        formatDialog.getFrameRate() );
  }
  else
  {
    m_pCurrStream->init( fileName, 0, 0, 0, 0 );
  }
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QApplication::restoreOverrideCursor();
  if( m_pCurrStream->getStatus() == 0 )
  {
    return false;
  }

  m_pCurrFrameQImage = new QImage( m_pCurrStream->getCurrFrame()->getQImageBuffer(), m_pCurrStream->getWidth(), m_pCurrStream->getHeight(), QImage::Format_RGB888 );

  m_cViewArea->setInputStream(m_pCurrStream);

  QApplication::restoreOverrideCursor();

  refreshFrame();
  //normalSize();

  m_cWindowName = QString( " " );
  m_cWindowName = m_pCurrStream->getStreamInformationString();

  m_cCurrFileName = fileName;

  setWindowTitle( m_cWindowName );

  return true;
}

/**
 * Functions to enable a module in the
 * current SubWindow
 */
Void SubWindowHandle::enableModule( PlaYUVerModuleIf* select_module )
{
  if( m_pcCurrentModule == select_module )
  {
    disableModule();
    return;
  }
  m_pcCurrentModule = select_module;
  m_pcCurrentModule->create( m_pCurrStream->getCurrFrame() );
  refreshFrame();
}

Void SubWindowHandle::disableModule()
{
  if( !m_pcCurrentModule )
    return;

  m_pcCurrentModule->destroy();
  m_pcCurrentModule = NULL;
  refreshFrame();
}

Void SubWindowHandle::refreshFrame()
{
  PlaYUVerFrame* CurrSubWindowFrame;
  if( m_pcCurrentModule )
  {
    CurrSubWindowFrame = m_pcCurrentModule->process( m_pCurrStream->getCurrFrame() );
  }
  else
  {
    CurrSubWindowFrame = m_pCurrStream->getCurrFrame();
  }
  CurrSubWindowFrame->FrametoRGB8();
  m_cViewArea->setImage( QPixmap::fromImage( QImage( CurrSubWindowFrame->getQImageBuffer(), CurrSubWindowFrame->getWidth(), CurrSubWindowFrame->getHeight(), QImage::Format_RGB888 ) ) );
}

Bool SubWindowHandle::save()
{
  QString supported = tr( "Supported Files" );
  QString formats = InputStream::supportedWriteFormats();
  formats.prepend( " (" );
  formats.append( ")" );
  supported.append( formats );  // supported=="Supported Files (*.pbm *.jpg...)"

  QStringList filter;
  filter << supported << InputStream::supportedWriteFormatsList() << tr( "All Files (*)" );

  QString fileName = QFileDialog::getSaveFileName( this, tr( "Save As" ), m_cCurrFileName, filter.join( ";;" ) );

  if( fileName.isEmpty() )
    return false;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  if( m_pCurrFrameQImage->save( fileName ) )
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::warning( this, tr( "plaYUVer" ), tr( "Cannot save file %1" ).arg( fileName ) );
    return false;
  }

  QApplication::restoreOverrideCursor();

  return true;
}

bool SubWindowHandle::playEvent()
{
  bool iRet = true;
  if( m_bIsPlaying )
  {
    m_pCurrStream->setNextFrame();
    refreshFrame();
    m_pCurrStream->readNextFrame();
    if( m_pCurrStream->checkErrors( READING ) )
    {
      QMessageBox::warning( this, tr( "plaYUVer" ), tr( "Cannot read %1." ).arg( m_cCurrFileName ) );
      return false;
    }
    else if( m_pCurrStream->checkErrors( END_OF_SEQ ) )
    {
      iRet = false;
    }
    return iRet;
  }
  return false;
}

Void SubWindowHandle::seekAbsoluteEvent( UInt new_frame_num )
{
  m_pCurrStream->seekInput( new_frame_num );
  refreshFrame();
}

Void SubWindowHandle::seekRelativeEvent( Bool bIsFoward )
{
  m_pCurrStream->seekInput( m_pCurrStream->getCurrFrameNum() + ( bIsFoward ? 1 : -1 ) );
  refreshFrame();
}

Void SubWindowHandle::stopEvent()
{
  seekAbsoluteEvent( 0 );
  return;
}

Void SubWindowHandle::normalSize()
{
  Double scaleFactor = 1.0;
  m_cViewArea->setZoomFactor( scaleFactor );
}

Void SubWindowHandle::zoomToFit()
{
  // Scale to a smaller size that the real to a nicer look
  QSize niceFit( m_cScrollArea->viewport()->size().width() - 5, m_cScrollArea->viewport()->size().height() - 5 );
  if( (Int)m_pCurrStream->getWidth() <= niceFit.width() && (Int)m_pCurrStream->getHeight() <= niceFit.height() )
  {
    normalSize();
    return;
  }
  scaleView( niceFit );
}

Void SubWindowHandle::scaleView( Double factor )
{
  Q_ASSERT( m_cViewArea->image() );
  m_cViewArea->zoomChangeEvent(factor);
}

Void SubWindowHandle::scaleView( Int width, Int height )
{
  scaleView( QSize( width, height ) );
}

Void SubWindowHandle::scaleView( const QSize & size )
{
  QSize imgViewSize( m_pCurrStream->getWidth(), m_pCurrStream->getHeight() );
  QSize newSize = imgViewSize;
  newSize.scale( size, Qt::KeepAspectRatio );

  // Calc the zoom factor
  Double wfactor = 1;
  Double hfactor = 1;

  wfactor = ( Double )newSize.width() / imgViewSize.width();
  hfactor = ( Double )newSize.height() / imgViewSize.height();

  if( wfactor < hfactor )
    scaleView( wfactor );
  else
    scaleView( hfactor );
}

void SubWindowHandle::adjustScrollBar( double factor )
{
  QScrollBar *scrollBar = m_cScrollArea->horizontalScrollBar();
  scrollBar->setValue( int( factor * scrollBar->value() + ( ( factor - 1 ) * scrollBar->pageStep() / 2 ) ) );
  scrollBar = m_cScrollArea->verticalScrollBar();
  scrollBar->setValue( int( factor * scrollBar->value() + ( ( factor - 1 ) * scrollBar->pageStep() / 2 ) ) );
}

QSize SubWindowHandle::sizeHint() const
{
  QSize maxSize;  // The size of the parent (viewport widget
                  // of the QMdiArea).

  QWidget *p = parentWidget();
  if( p )
  {
    maxSize = p->size();
  }

  QSize isize = QSize( m_pCurrStream->getWidth() + 50, m_pCurrStream->getHeight() + 50 );

  // If the SubWindowHandle needs more space that the avaiable, we'll give
  // to the subwindow a reasonable size preserving the image aspect ratio.
  if( isize.width() < maxSize.width() && isize.height() < maxSize.height() )
  {
    return isize;
  }
  else
  {
    isize.scale( maxSize, Qt::KeepAspectRatio );
    maxSize = isize;
  }

  return maxSize;
}

Bool SubWindowHandle::mayClose()
{
  return true;
}

void SubWindowHandle::closeEvent( QCloseEvent *event )
{
  event->accept();
}

}  // NAMESPACE