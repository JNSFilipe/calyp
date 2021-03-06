/*    This file is a part of Calyp project
 *    Copyright (C) 2014-2019  by Joao Carreira   (jfmcarreira@gmail.com)
 *                                Luis Lucas      (luisfrlucas@gmail.com)
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
 * \file     VideoHandle.h
 * \brief    Class to handle video playback
 */

#include "VideoHandle.h"

#include "FrameNumberWidget.h"
#include "FramePropertiesDock.h"
#include "MainWindow.h"
#include "SeekStreamDialog.h"
#include "SubWindowHandle.h"
#include "SubWindowSelectorDialog.h"
#include "VideoSubWindow.h"

#include <QDockWidget>
#include <QElapsedTimer>
#include <QLabel>
#include <QSignalMapper>
#include <QSlider>
#include <QToolBar>
#include <QtGui>
#if( _CONTROL_PLAYING_TIME_ == 1 )
#include <QElapsedTimer>
#endif

VideoHandle::VideoHandle( QWidget* parent, SubWindowHandle* windowManager )
    : m_pcParet( parent ), m_pcMainWindowManager( windowManager )
{
  m_pcCurrentVideoSubWindow = NULL;
  m_bIsPlaying = false;
  m_pcPlayingTimer = new QTimer;
  m_pcPlayingTimer->setTimerType( Qt::PreciseTimer );
  connect( m_pcPlayingTimer, SIGNAL( timeout() ), this, SLOT( playEvent() ) );
  m_acPlayingSubWindows.clear();
}

VideoHandle::~VideoHandle() {}

void VideoHandle::createActions()
{
  m_arrayActions.resize( TOTAL_ACT );
  m_arrayActions[PLAY_ACT] = new QAction( "Play", this );
  m_arrayActions[PLAY_ACT]->setStatusTip( "Play/Pause" );
  m_arrayActions[PLAY_ACT]->setIcon( QIcon( m_pcParet->style()->standardIcon( QStyle::SP_MediaPlay ) ) );
  m_arrayActions[PLAY_ACT]->setShortcut( Qt::Key_Space );
  connect( m_arrayActions[PLAY_ACT], SIGNAL( triggered() ), this, SLOT( play() ) );

  m_arrayActions[STOP_ACT] = new QAction( "Stop", this );
  m_arrayActions[STOP_ACT]->setStatusTip( "Stop" );
  m_arrayActions[STOP_ACT]->setIcon( QIcon( m_pcParet->style()->standardIcon( QStyle::SP_MediaStop ) ) );
  connect( m_arrayActions[STOP_ACT], SIGNAL( triggered() ), this, SLOT( stop() ) );

  m_mapperVideoSeek = new QSignalMapper( this );
  connect( m_mapperVideoSeek, SIGNAL( mapped( int ) ), this, SLOT( seekEvent( int ) ) );

  m_arrayActions[VIDEO_BACKWARD_ACT] = new QAction( "Video Backward", this );
  m_arrayActions[VIDEO_BACKWARD_ACT]->setStatusTip( "Seek backward" );
  m_arrayActions[VIDEO_BACKWARD_ACT]->setIcon(
      QIcon( m_pcParet->style()->standardIcon( QStyle::SP_MediaSeekBackward ) ) );
  m_arrayActions[VIDEO_BACKWARD_ACT]->setShortcut( Qt::Key_Left );
  connect( m_arrayActions[VIDEO_BACKWARD_ACT], SIGNAL( triggered() ), m_mapperVideoSeek, SLOT( map() ) );
  m_mapperVideoSeek->setMapping( m_arrayActions[VIDEO_BACKWARD_ACT], 0 );

  m_arrayActions[VIDEO_FORWARD_ACT] = new QAction( "Video Forward", this );
  m_arrayActions[VIDEO_FORWARD_ACT]->setStatusTip( "Seek forward" );
  m_arrayActions[VIDEO_FORWARD_ACT]->setIcon(
      QIcon( m_pcParet->style()->standardIcon( QStyle::SP_MediaSeekForward ) ) );
  m_arrayActions[VIDEO_FORWARD_ACT]->setShortcut( Qt::Key_Right );
  connect( m_arrayActions[VIDEO_FORWARD_ACT], SIGNAL( triggered() ), m_mapperVideoSeek, SLOT( map() ) );
  m_mapperVideoSeek->setMapping( m_arrayActions[VIDEO_FORWARD_ACT], 1 );

  m_arrayActions[VIDEO_GOTO_ACT] = new QAction( "Go to", this );
  m_arrayActions[VIDEO_GOTO_ACT]->setStatusTip( "Seek to a specific frame number" );
  m_arrayActions[VIDEO_GOTO_ACT]->setShortcut( tr( "Ctrl+G" ) );
  connect( m_arrayActions[VIDEO_GOTO_ACT], SIGNAL( triggered() ), this, SLOT( seekVideo() ) );

  m_arrayActions[VIDEO_REPEAT_ACT] = new QAction( "Repeat", this );
  m_arrayActions[VIDEO_REPEAT_ACT]->setStatusTip( "Enable loop through the sequence" );
  m_arrayActions[VIDEO_REPEAT_ACT]->setShortcut( tr( "Ctrl+R" ) );
  m_arrayActions[VIDEO_REPEAT_ACT]->setCheckable( true );
  m_arrayActions[VIDEO_REPEAT_ACT]->setChecked( false );

  m_arrayActions[VIDEO_ZOOM_LOCK_ACT] = new QAction( "Zoom Lock", this );
  m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->setStatusTip( "Sync zoom between all video windows" );
  m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->setShortcut( tr( "Ctrl+L" ) );
  m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->setCheckable( true );
  m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->setChecked( false );

  m_arrayActions[VIDEO_LOCK_SELECTION_ACT] = new QAction( "Sync Play", this );
  m_arrayActions[VIDEO_LOCK_SELECTION_ACT]->setStatusTip(
      "Select which windows should be played synchronized. Press Ctrl while "
      "click to quickly to select all!" );
  connect( m_arrayActions[VIDEO_LOCK_SELECTION_ACT], SIGNAL( triggered() ), this, SLOT( videoSelectionButtonEvent() ) );

  m_pcFrameSlider = new QSlider;
  m_pcFrameSlider->setOrientation( Qt::Horizontal );
  m_pcFrameSlider->setMaximumWidth( 2000 );
  m_pcFrameSlider->setSizePolicy( QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed ) );
  m_pcFrameSlider->setEnabled( false );
  m_pcFrameSlider->setTracking( false );
  connect( m_pcFrameSlider, SIGNAL( valueChanged( int ) ), this, SLOT( seekSliderEvent( int ) ) );

  // ------------ Image ------------
  m_actionGroupTools = new QActionGroup( this );
  m_actionGroupTools->setExclusive( true );

  m_mapperTools = new QSignalMapper( this );
  connect( m_mapperTools, SIGNAL( mapped( int ) ), this, SLOT( setTool( int ) ) );

  m_uiViewTool = ViewArea::NavigationView;

  m_arrayActions[NAVIGATION_TOOL_ACT] = new QAction( tr( "Navigation Tool" ), this );
  m_arrayActions[NAVIGATION_TOOL_ACT]->setCheckable( true );
  m_arrayActions[NAVIGATION_TOOL_ACT]->setChecked( true );
  m_arrayActions[NAVIGATION_TOOL_ACT]->setShortcut( Qt::CTRL + Qt::Key_1 );
  m_actionGroupTools->addAction( m_arrayActions[NAVIGATION_TOOL_ACT] );
  connect( m_arrayActions[NAVIGATION_TOOL_ACT], SIGNAL( triggered() ), m_mapperTools, SLOT( map() ) );
  m_mapperTools->setMapping( m_arrayActions[NAVIGATION_TOOL_ACT], ViewArea::NavigationView );

  m_arrayActions[SELECTION_TOOL_ACT] = new QAction( "Selection Tool", this );
  m_arrayActions[SELECTION_TOOL_ACT]->setCheckable( true );
  m_arrayActions[SELECTION_TOOL_ACT]->setChecked( false );
  m_arrayActions[SELECTION_TOOL_ACT]->setShortcut( Qt::CTRL + Qt::Key_2 );
  m_actionGroupTools->addAction( m_arrayActions[SELECTION_TOOL_ACT] );
  connect( m_arrayActions[SELECTION_TOOL_ACT], SIGNAL( triggered() ), m_mapperTools, SLOT( map() ) );
  m_mapperTools->setMapping( m_arrayActions[SELECTION_TOOL_ACT], ViewArea::NormalSelectionView );

  m_arrayActions[BLOCK_SELECTION_TOOL_ACT] = new QAction( "Block Selection Tool", this );
  m_arrayActions[BLOCK_SELECTION_TOOL_ACT]->setCheckable( true );
  m_arrayActions[BLOCK_SELECTION_TOOL_ACT]->setChecked( false );
  m_arrayActions[BLOCK_SELECTION_TOOL_ACT]->setShortcut( Qt::CTRL + Qt::Key_3 );
  m_actionGroupTools->addAction( m_arrayActions[BLOCK_SELECTION_TOOL_ACT] );
  connect( m_arrayActions[BLOCK_SELECTION_TOOL_ACT], SIGNAL( triggered() ), m_mapperTools, SLOT( map() ) );
  m_mapperTools->setMapping( m_arrayActions[BLOCK_SELECTION_TOOL_ACT], ViewArea::BlockSelectionView );

  m_mapperGrid = new QSignalMapper( this );
  m_actionGroupGrid = new QActionGroup( this );
  m_actionGroupGrid->setExclusive( true );
  m_mapperGrid->setMapping( m_actionGroupGrid->addAction( "8x8" ), 8 );
  m_mapperGrid->setMapping( m_actionGroupGrid->addAction( "16x16" ), 16 );
  m_mapperGrid->setMapping( m_actionGroupGrid->addAction( "64x64" ), 64 );

  for( int i = 0; i < m_actionGroupGrid->actions().size(); i++ )
  {
    m_actionGroupGrid->actions().at( i )->setCheckable( true );
    connect( m_actionGroupGrid->actions().at( i ), SIGNAL( triggered() ), m_mapperGrid, SLOT( map() ) );
  }
  m_actionGroupGrid->actions().at( 2 )->setChecked( true );
  connect( m_mapperGrid, SIGNAL( mapped( int ) ), this, SLOT( setGridSize( int ) ) );

  m_arrayActions[SHOW_GRID_ACT] = new QAction( "Show grid", this );
  m_arrayActions[SHOW_GRID_ACT]->setCheckable( true );
  m_arrayActions[SHOW_GRID_ACT]->setChecked( false );
  connect( m_arrayActions[SHOW_GRID_ACT], SIGNAL( toggled( bool ) ), this, SLOT( toggleGrid( bool ) ) );
}

QMenu* VideoHandle::createVideoMenu()
{
  m_pcMenuVideo = new QMenu( "Video", m_pcParet );
  m_pcMenuVideo->addAction( m_arrayActions[PLAY_ACT] );
  m_pcMenuVideo->addAction( m_arrayActions[STOP_ACT] );
  m_pcMenuVideo->addAction( m_arrayActions[VIDEO_BACKWARD_ACT] );
  m_pcMenuVideo->addAction( m_arrayActions[VIDEO_FORWARD_ACT] );
  m_pcMenuVideo->addAction( m_arrayActions[VIDEO_GOTO_ACT] );
  m_pcMenuVideo->addSeparator();
  m_pcMenuVideo->addAction( m_arrayActions[VIDEO_REPEAT_ACT] );
  m_pcMenuVideo->addAction( m_arrayActions[VIDEO_ZOOM_LOCK_ACT] );
  m_pcMenuVideo->addAction( m_arrayActions[VIDEO_LOCK_SELECTION_ACT] );
  return m_pcMenuVideo;
}

QMenu* VideoHandle::createImageMenu()
{
  m_pcMenuImage = new QMenu( "Image", m_pcParet );
  m_pcMenuImage->addAction( m_arrayActions[NAVIGATION_TOOL_ACT] );
  m_pcMenuImage->addAction( m_arrayActions[SELECTION_TOOL_ACT] );
  m_pcMenuImage->addAction( m_arrayActions[BLOCK_SELECTION_TOOL_ACT] );
  m_pcMenuImage->addSeparator();
  m_pcMenuImage->addAction( m_arrayActions[SHOW_GRID_ACT] );
  QMenu* gridSizeMenu = m_pcMenuImage->addMenu( "Grid size" );
  gridSizeMenu->addActions( m_actionGroupGrid->actions() );
  return m_pcMenuImage;
}

QToolBar* VideoHandle::createToolBar()
{
  m_toolbarVideo = new QToolBar( tr( "Video" ) );
  m_toolbarVideo->setAllowedAreas( Qt::TopToolBarArea | Qt::BottomToolBarArea );
  m_toolbarVideo->addAction( m_arrayActions[PLAY_ACT] );
  m_toolbarVideo->addAction( m_arrayActions[STOP_ACT] );

  QLabel* pcLockLabel = new QLabel( "PlayingLock" );
  pcLockLabel->setAlignment( Qt::AlignCenter );
  m_arrayActions[VIDEO_LOCK_ACT] = m_toolbarVideo->addWidget( pcLockLabel );
  m_arrayActions[VIDEO_LOCK_ACT]->setVisible( false );

  m_toolbarVideo->addAction( m_arrayActions[VIDEO_BACKWARD_ACT] );
  m_toolbarVideo->addWidget( m_pcFrameSlider );
  m_toolbarVideo->addAction( m_arrayActions[VIDEO_FORWARD_ACT] );
  m_toolbarVideo->addWidget( new QLabel );
  m_pcFrameNumInfo = new FrameNumberWidget;
  m_toolbarVideo->addWidget( m_pcFrameNumInfo );

  return m_toolbarVideo;
}

QDockWidget* VideoHandle::createDock()
{
  m_pcFramePropertiesSideBar = new FramePropertiesDock( m_pcParet, &m_bIsPlaying );
  m_pcFramePropertiesDock = new QDockWidget( tr( "Frame Information" ), m_pcParet );
  m_pcFramePropertiesDock->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
  m_pcFramePropertiesDock->setWidget( m_pcFramePropertiesSideBar );
  connect( m_pcFramePropertiesDock, SIGNAL( visibilityChanged( bool ) ), this, SLOT( update() ) );

  return m_pcFramePropertiesDock;
}

QWidget* VideoHandle::createStatusBarMessage()
{
  QWidget* pcStatusBarWidget = new QWidget;
  QHBoxLayout* mainlayout = new QHBoxLayout;

  m_pcVideoFormatLabel = new QLabel;
  m_pcVideoFormatLabel->setText( " " );
  m_pcVideoFormatLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  m_pcVideoFormatLabel->setMinimumWidth( 150 );
  m_pcVideoFormatLabel->setAlignment( Qt::AlignCenter );

  m_pcResolutionLabel = new QLabel;
  m_pcResolutionLabel->setText( " " );
  m_pcResolutionLabel->setSizePolicy( QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed ) );
  m_pcResolutionLabel->setMinimumWidth( 90 );
  m_pcResolutionLabel->setAlignment( Qt::AlignCenter );

  mainlayout->addWidget( m_pcVideoFormatLabel );
  mainlayout->addWidget( m_pcResolutionLabel );
  pcStatusBarWidget->setLayout( mainlayout );

  return pcStatusBarWidget;
}

void VideoHandle::updateMenus()
{
  VideoSubWindow* pcSubWindow = qobject_cast<VideoSubWindow*>( m_pcMainWindowManager->activeSubWindow() );
  bool hasSubWindow = pcSubWindow ? true : false;

  m_arrayActions[PLAY_ACT]->setEnabled( hasSubWindow );
  // m_arrayActions[PAUSE_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[STOP_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[VIDEO_BACKWARD_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[VIDEO_FORWARD_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[VIDEO_GOTO_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[VIDEO_LOCK_SELECTION_ACT]->setEnabled( hasSubWindow );
  m_pcFrameSlider->setEnabled( hasSubWindow );
  if( !hasSubWindow )
  {
    disconnect( m_pcFrameSlider, SIGNAL( valueChanged( int ) ), this, SLOT( seekSliderEvent( int ) ) );
    m_pcFrameSlider->setValue( 0 );
    m_pcFrameNumInfo->clear();
  }
  else
  {
    connect( m_pcFrameSlider, SIGNAL( valueChanged( int ) ), this, SLOT( seekSliderEvent( int ) ) );
  }

  m_arrayActions[NAVIGATION_TOOL_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[SELECTION_TOOL_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[BLOCK_SELECTION_TOOL_ACT]->setEnabled( hasSubWindow );
  m_arrayActions[SHOW_GRID_ACT]->setEnabled( hasSubWindow );
}

void VideoHandle::readSettings()
{
  QSettings appSettings;
  m_arrayActions[VIDEO_REPEAT_ACT]->setChecked( appSettings.value( "VideoHandle/Repeat", false ).toBool() );
  m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->setChecked( appSettings.value( "VideoHandle/VideoZoomLock", false ).toBool() );
  if( !appSettings.value( "VideoHandle/FrameProperties", true ).toBool() )
    m_pcFramePropertiesDock->close();

  m_uiViewTool = appSettings.value( "VideoHandle/SelectedTool", ViewArea::NavigationView ).toUInt();
  setTool( m_uiViewTool );

  m_arrayActions[SHOW_GRID_ACT]->setChecked( appSettings.value( "VideoHandle/ShowGrid", false ).toBool() );
}

void VideoHandle::writeSettings()
{
  QSettings appSettings;
  appSettings.setValue( "VideoHandle/Repeat", m_arrayActions[VIDEO_REPEAT_ACT]->isChecked() );
  appSettings.setValue( "VideoHandle/VideoZoomLock", m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->isChecked() );
  appSettings.setValue( "VideoHandle/FrameProperties", m_pcFramePropertiesSideBar->isVisible() );
  appSettings.setValue( "VideoHandle/SelectedTool", m_uiViewTool );
  appSettings.setValue( "VideoHandle/ShowGrid", m_arrayActions[SHOW_GRID_ACT]->isChecked() );
}

void VideoHandle::update()
{
  if( m_pcCurrentVideoSubWindow )
  {
    CalypStream* pcStream = m_pcCurrentVideoSubWindow->getInputStream();
    CalypFrame* pcFrame = m_pcCurrentVideoSubWindow->getCurrFrame();

    unsigned long frame_num = 0;
    unsigned long total_frame_num = 1;

    m_pcVideoFormatLabel->setText( m_pcCurrentVideoSubWindow->getStreamInformation() );

    QString resolution;
    if( pcFrame )
    {
      resolution.append( QString( "%1x%2" ).arg( pcFrame->getWidth() ).arg( pcFrame->getHeight() ) );
    }

    if( pcStream )
    {
      frame_num = pcStream->getCurrFrameNum();
      m_pcFrameNumInfo->setCurrFrameNum( frame_num );
      total_frame_num = getMaxFrameNumber();
      resolution.append( QString( "@%1" ).arg( pcStream->getFrameRate() ) );
    }

    if( pcFrame )
    {
      resolution.append( QString( " %1 bpp" ).arg( pcFrame->getBitsPel() ) );
    }
    if( pcStream && pcFrame->getBitsPel() > 8 )
    {
      if( pcStream->getEndianess() == CLP_BIG_ENDIAN )
      {
        resolution.append( QStringLiteral( " (BE)" ) );
      }
      else
      {
        resolution.append( QStringLiteral( " (LE)" ) );
      }
    }
    m_pcResolutionLabel->setText( resolution );

    m_pcFramePropertiesSideBar->setFrame( pcFrame );

    m_pcFrameNumInfo->setTotalFrameNum( total_frame_num );
    m_pcFrameNumInfo->setCurrFrameNum( frame_num );

    m_pcFrameSlider->blockSignals( true );  // Disable signals otherwise it will loop this function
    m_pcFrameSlider->setMaximum( total_frame_num - 1 );
    m_pcFrameSlider->setValue( frame_num );
    m_pcFrameSlider->blockSignals( false );

    if( m_pcCurrentVideoSubWindow->isPlaying() )
    {
      m_arrayActions[PLAY_ACT]->setText( "Pause" );
      m_arrayActions[PLAY_ACT]->setIcon( m_pcParet->style()->standardIcon( QStyle::SP_MediaPause ) );
    }
    else
    {
      m_arrayActions[PLAY_ACT]->setText( "Play" );
      m_arrayActions[PLAY_ACT]->setIcon( m_pcParet->style()->standardIcon( QStyle::SP_MediaPlay ) );
    }
  }
  else
  {
    m_pcVideoFormatLabel->setText( "" );
    m_pcResolutionLabel->setText( "" );
    m_pcFramePropertiesSideBar->reset();
    m_pcFrameSlider->setValue( 0 );
    m_pcFrameNumInfo->clear();
  }
}

void VideoHandle::update( VideoSubWindow* currSubWindow )
{
  m_pcCurrentVideoSubWindow = currSubWindow;
  update();
}

void VideoHandle::updateSelectionArea( QRect area )
{
  if( m_pcCurrentVideoSubWindow )
  {
    m_pcFramePropertiesSideBar->setSelection( area );
  }
}

void VideoHandle::addSubWindow( VideoSubWindow* window )
{
  window->zoomToFit();
  window->getViewArea()->setTool( m_uiViewTool );
  window->getViewArea()->setGridVisible( m_arrayActions[SHOW_GRID_ACT]->isChecked() );

  connect( window, &SubWindowAbstract::aboutToClose,
           this, &VideoHandle::closeSubWindow );

  connect( window, &SubWindowAbstract::zoomFactorChanged,
           this, &VideoHandle::zoomToFactorAll );

  connect( window, &SubWindowAbstract::scrollBarMoved,
           this, &VideoHandle::moveAllScrollBars );

  connect( window->getViewArea(), SIGNAL( selectionChanged( QRect ) ), this, SLOT( updateSelectionArea( QRect ) ) );
}

void VideoHandle::closeSubWindow( SubWindowAbstract* subWindow )
{
  VideoSubWindow* videoSubWindow = qobject_cast<VideoSubWindow*>( subWindow );

  if( m_acPlayingSubWindows.contains( videoSubWindow ) )
  {
    int pos = m_acPlayingSubWindows.indexOf( videoSubWindow );
    m_acPlayingSubWindows.at( pos )->stop();
    m_acPlayingSubWindows.remove( pos );
    if( m_acPlayingSubWindows.size() < 2 )
    {
      m_arrayActions[VIDEO_LOCK_ACT]->setVisible( false );
    }
  }
}

void VideoHandle::zoomToFactorAll( const double scale, const QPoint center )
{
  double factor;

  if( m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->isChecked() )
  {
    factor = m_pcCurrentVideoSubWindow->getScaleFactor();

    SubWindowAbstract* subWindow;
    QList<SubWindowAbstract*> subWindowList =
        m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );
    for( int i = 0; i < subWindowList.size(); i++ )
    {
      subWindow = subWindowList.at( i );
      if( m_pcCurrentVideoSubWindow == subWindow )
        continue;
      else
      {
        subWindow->zoomToFactor( factor, center );
      }
    }
  }
}

#if 0
void VideoHandle::moveAllScrollBars( const QPoint& offset )
{
  QPoint newOffset = offset;
  if( m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->isChecked() && m_pcCurrentVideoSubWindow )
  {
    VideoSubWindow* videoSubWindow;
    QList<SubWindowAbstract*> subWindowList =
        m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );

    QScrollBar* scrollBar;
    scrollBar = m_pcCurrentVideoSubWindow->getScroll()->horizontalScrollBar();

    // Do not move other images, if current image reached maximum or minimum
    // scroll position
    if( scrollBar->value() == scrollBar->maximum() && offset.x() > 0 )
      newOffset.setX( 0 );
    if( scrollBar->value() == scrollBar->minimum() && offset.x() < 0 )
      newOffset.setX( 0 );

    scrollBar = m_pcCurrentVideoSubWindow->getScroll()->verticalScrollBar();
    if( scrollBar->value() == scrollBar->maximum() && offset.y() > 0 )
      newOffset.setY( 0 );
    if( scrollBar->value() == scrollBar->minimum() && offset.y() < 0 )
      newOffset.setY( 0 );

    for( int i = 0; i < subWindowList.size(); i++ )
    {
      videoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
      if( m_pcCurrentVideoSubWindow == videoSubWindow )
        continue;
      else
      {
        videoSubWindow->adjustScrollBarByOffset( newOffset );
      }
    }
  }
}
#else
void VideoHandle::moveAllScrollBars( const double& horRatio, const double& verRatio )
{
  if( m_arrayActions[VIDEO_ZOOM_LOCK_ACT]->isChecked() && m_pcCurrentVideoSubWindow )
  {
    VideoSubWindow* videoSubWindow;
    QList<SubWindowAbstract*> subWindowList = m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );

    for( int i = 0; i < subWindowList.size(); i++ )
    {
      videoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
      if( m_pcCurrentVideoSubWindow == videoSubWindow )
        continue;
      else
      {
        videoSubWindow->adjustScrollBarToRatio( horRatio, verRatio );
      }
    }
  }
}
#endif

unsigned long VideoHandle::getMaxFrameNumber()
{
  unsigned long currFrames;
  unsigned long maxFrames = INT_MAX;
  if( m_pcCurrentVideoSubWindow )
  {
    if( m_acPlayingSubWindows.contains( m_pcCurrentVideoSubWindow ) )
    {
      for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
      {
        currFrames = m_acPlayingSubWindows.at( i )->getInputStream()->getFrameNum();
        if( currFrames < maxFrames )
          maxFrames = currFrames;
      }
    }
    else
    {
      maxFrames = m_pcCurrentVideoSubWindow->getInputStream()->getFrameNum();
    }
  }
  return maxFrames;
}

void VideoHandle::setTimerStatus()
{
  bool status = false;
  for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
  {
    status |= m_acPlayingSubWindows.at( i )->isPlaying();
  }
  if( m_pcCurrentVideoSubWindow )
  {
    status |= m_pcCurrentVideoSubWindow->isPlaying();
  }
  if( status )
  {
    if( !m_bIsPlaying )
    {
      m_pcPlayingTimer->start();
      m_bIsPlaying = true;
#if( _CONTROL_PLAYING_TIME_ == 1 )
      m_uiNumberPlayedFrames = 0;
      m_pcPlayControlTimer = new QElapsedTimer;
      m_pcPlayControlTimer->start();
      m_dAverageFps = 0;
#endif
    }
  }
  else
  {
#if( _CONTROL_PLAYING_TIME_ == 1 )
    if( m_pcPlayControlTimer && m_pcPlayingTimer->interval() > 0 )
    {
      qDebug() << "Desired Fps: " << 1000 / m_pcPlayingTimer->interval() << "Real Fps: " << 1000 / m_dAverageFps;

      delete m_pcPlayControlTimer;
      m_pcPlayControlTimer = NULL;
    }
#endif
    m_pcPlayingTimer->stop();
    m_bIsPlaying = false;
  }
}

void VideoHandle::play()
{
  if( !m_pcCurrentVideoSubWindow )
    return;

  if( !m_pcCurrentVideoSubWindow->getInputStream() )
    return;

  if( !m_pcCurrentVideoSubWindow->isPlaying() )  // Not playing
  {
    double frameRate;
    unsigned int timeinterval;
    if( m_acPlayingSubWindows.size() < 2 )
    {
      for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
      {
        m_acPlayingSubWindows.at( i )->pause();
      }
      m_acPlayingSubWindows.clear();
      m_acPlayingSubWindows.append( m_pcCurrentVideoSubWindow );
    }
    if( m_acPlayingSubWindows.contains( m_pcCurrentVideoSubWindow ) )
    {
      for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
      {
        m_acPlayingSubWindows.at( i )->play();
      }
      frameRate = m_acPlayingSubWindows.at( 0 )->getInputStream()->getFrameRate();
      timeinterval = (unsigned int)( 1000.0 / frameRate + 0.5 );
      m_pcPlayingTimer->setInterval( timeinterval );
    }
  }
  else
  {
    if( m_acPlayingSubWindows.contains( m_pcCurrentVideoSubWindow ) )
    {
      for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
      {
        m_acPlayingSubWindows.at( i )->pause();
      }
    }
    else
    {
      m_pcCurrentVideoSubWindow->pause();
    }
  }
  setTimerStatus();
  emit changed();
  // update();
}

void VideoHandle::stop()
{
  if( m_acPlayingSubWindows.size() > 0 )
  {
    for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
    {
      m_acPlayingSubWindows.at( i )->stop();
    }
    m_acPlayingSubWindows.clear();
    setTimerStatus();
  }
  else
  {
    if( m_pcCurrentVideoSubWindow )
    {
      m_pcCurrentVideoSubWindow->stop();
    }
  }
  emit changed();
  m_arrayActions[VIDEO_LOCK_ACT]->setVisible( false );
}

void VideoHandle::playEvent()
{
  bool bEndOfSequence = false;
#if( _CONTROL_PLAYING_TIME_ == 1 )
  m_dAverageFps = double( m_dAverageFps * m_uiNumberPlayedFrames + m_pcPlayControlTimer->elapsed() ) /
                  double( m_uiNumberPlayedFrames + 1 );
  m_uiNumberPlayedFrames++;
  m_pcPlayControlTimer->restart();
#endif
  try
  {
    for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
    {
      bEndOfSequence |= m_acPlayingSubWindows.at( i )->playEvent();
    }
    if( bEndOfSequence )
    {
      if( !m_arrayActions[VIDEO_REPEAT_ACT]->isChecked() )
        stop();
      else
        for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
          m_acPlayingSubWindows.at( i )->seekAbsoluteEvent( 0 );
    }
  }
  catch( const char* msg )
  {
    QString warningMsg = "Error while playing " +
                         QFileInfo( m_pcCurrentVideoSubWindow->getCurrentFileName() ).fileName() +
                         " with the following error: \n" + msg;
    QMessageBox::warning( m_pcParet, QApplication::applicationName(), warningMsg );
    qDebug() << warningMsg;
    stop();
    m_pcCurrentVideoSubWindow->close();
  }
  emit changed();
}

void VideoHandle::seekEvent( int direction )
{
  if( m_pcCurrentVideoSubWindow )
  {
    if( m_acPlayingSubWindows.contains( m_pcCurrentVideoSubWindow ) )
    {
      if( !( (unsigned int)( m_pcCurrentVideoSubWindow->getInputStream()->getCurrFrameNum() + 1 ) >= getMaxFrameNumber() &&
             direction > 0 ) )
      {
        for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
        {
          m_acPlayingSubWindows.at( i )->seekRelativeEvent( direction > 0 ? true : false );
        }
      }
    }
    else
    {
      m_pcCurrentVideoSubWindow->seekRelativeEvent( direction > 0 ? true : false );
    }
    emit changed();
    // update();
  }
}

void VideoHandle::seekVideo()
{
  if( m_pcCurrentVideoSubWindow )
  {
    if( m_pcCurrentVideoSubWindow->getInputStream() )
    {
      SeekStreamDialog* dialogSeekVideo =
          new SeekStreamDialog( m_pcCurrentVideoSubWindow->getInputStream(), m_pcParet );
      int newFrameNum = dialogSeekVideo->runDialog();
      if( newFrameNum >= 0 )
      {
        if( m_acPlayingSubWindows.contains( m_pcCurrentVideoSubWindow ) )
        {
          for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
          {
            m_acPlayingSubWindows.at( i )->seekAbsoluteEvent( (unsigned int)newFrameNum );
          }
        }
        else
        {
          m_pcCurrentVideoSubWindow->seekAbsoluteEvent( (unsigned int)newFrameNum );
        }
        emit changed();
      }
    }
  }
}

void VideoHandle::seekSliderEvent( int new_frame_num )
{
  if( m_pcCurrentVideoSubWindow && !m_bIsPlaying )  // TODO: Fix this slot
  {
    if( m_acPlayingSubWindows.contains( m_pcCurrentVideoSubWindow ) )
    {
      for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
      {
        m_acPlayingSubWindows.at( i )->seekAbsoluteEvent( (unsigned int)new_frame_num );
      }
    }
    else
    {
      m_pcCurrentVideoSubWindow->seekAbsoluteEvent( (unsigned int)new_frame_num );
    }
    emit changed();
  }
}

void VideoHandle::videoSelectionButtonEvent()
{
  VideoSubWindow* videoSubWindow;

  Qt::KeyboardModifiers keyModifiers = QApplication::keyboardModifiers();
  if( keyModifiers & Qt::ControlModifier )
  {
    QList<SubWindowAbstract*> subWindowList =
        m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_STREAM_SUBWINDOW );
    for( int i = 0; i < subWindowList.size(); i++ )
    {
      videoSubWindow = qobject_cast<VideoSubWindow*>( subWindowList.at( i ) );
      m_acPlayingSubWindows.append( videoSubWindow );
    }
  }
  else
  {
    SubWindowSelectorDialog dialogWindowsSelection( m_pcParet, m_pcMainWindowManager,
                                                    SubWindowAbstract::VIDEO_STREAM_SUBWINDOW );
    QStringList cWindowListNames;
    for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
    {
      dialogWindowsSelection.selectSubWindow( m_acPlayingSubWindows.at( i ) );
    }
    if( dialogWindowsSelection.exec() == QDialog::Accepted )
    {
      m_acPlayingSubWindows.clear();

      QList<SubWindowAbstract*> selectedSubWindowList = dialogWindowsSelection.getSelectedWindows();
      for( int i = 0; i < selectedSubWindowList.size(); i++ )
      {
        videoSubWindow = qobject_cast<VideoSubWindow*>( selectedSubWindowList.at( i ) );
        m_acPlayingSubWindows.append( videoSubWindow );
      }
    }
    else
    {
      return;
    }
  }

  if( m_acPlayingSubWindows.size() > 0 )
  {
    m_acPlayingSubWindows.at( 0 )->activateWindow();
    if( m_acPlayingSubWindows.size() > 1 )
    {
      m_arrayActions[VIDEO_LOCK_ACT]->setVisible( true );
    }
    if( m_bIsPlaying )
    {
      for( int i = 0; i < m_acPlayingSubWindows.size(); i++ )
      {
        m_acPlayingSubWindows.at( i )->play();
      }
    }
  }
  else
  {
    stop();
  }
}

void VideoHandle::setTool( int tool )
{
  m_uiViewTool = tool;
  m_actionGroupTools->actions().at( m_uiViewTool )->setChecked( true );
  QList<SubWindowAbstract*> subWindowList = m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );
  for( int i = 0; i < subWindowList.size(); i++ )
  {
    qobject_cast<VideoSubWindow*>( subWindowList.at( i ) )->getViewArea()->setTool( m_uiViewTool );
  }
}

void VideoHandle::toggleGrid( bool checked )
{
  QList<SubWindowAbstract*> subWindowList = m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );
  for( int i = 0; i < subWindowList.size(); i++ )
  {
    qobject_cast<VideoSubWindow*>( subWindowList.at( i ) )->getViewArea()->setGridVisible( checked );
  }
}

void VideoHandle::setGridSize( int size )
{
  QList<SubWindowAbstract*> subWindowList = m_pcMainWindowManager->findSubWindow( SubWindowAbstract::VIDEO_SUBWINDOW );
  for( int i = 0; i < subWindowList.size(); i++ )
  {
    qobject_cast<VideoSubWindow*>( subWindowList.at( i ) )->getViewArea()->setGridSize( size );
  }
}
