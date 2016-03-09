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
 * \file     PlaYUVerStream.cpp
 * \ingroup  PlaYUVerLib
 * \brief    Input stream handling
 */

#include "config.h"
#include <cstdio>
#include <vector>
#include "LibMemory.h"
#include "PlaYUVerStream.h"
#ifdef USE_FFMPEG
#include "LibAvContextHandle.h"
#endif
#ifdef USE_OPENCV
#include "LibOpenCVHandler.h"
#endif

namespace plaYUVer
{

std::vector<PlaYUVerSupportedFormat> PlaYUVerStream::supportedReadFormats()
{
  INI_REGIST_PLAYUVER_SUPPORTED_FMT;
  REGIST_PLAYUVER_SUPPORTED_FMT( "Raw Video", "yuv" );
#ifdef USE_FFMPEG
  APPEND_PLAYUVER_SUPPORTED_FMT( LibAvContextHandle, Read );
#endif
//#ifdef USE_OPENCV
//  APPEND_PLAYUVER_SUPPORTED_FMT( LibOpenCVHandler );
//#endif
  END_REGIST_PLAYUVER_SUPPORTED_FMT;
}

std::vector<PlaYUVerSupportedFormat> PlaYUVerStream::supportedWriteFormats()
{
  INI_REGIST_PLAYUVER_SUPPORTED_FMT;
  REGIST_PLAYUVER_SUPPORTED_FMT( "Raw Video", "yuv" );
#ifdef USE_FFMPEG
  APPEND_PLAYUVER_SUPPORTED_FMT( LibAvContextHandle, Write );
#endif
  END_REGIST_PLAYUVER_SUPPORTED_FMT;
}

std::vector<PlaYUVerSupportedFormat> PlaYUVerStream::supportedSaveFormats()
{
  INI_REGIST_PLAYUVER_SUPPORTED_FMT;
  APPEND_PLAYUVER_SUPPORTED_FMT( PlaYUVerStream, Write );
#ifdef USE_FFMPEG
  APPEND_PLAYUVER_SUPPORTED_FMT( LibAvContextHandle, Save );
#endif
#ifdef USE_OPENCV
  APPEND_PLAYUVER_SUPPORTED_FMT( LibOpenCVHandler, Save );
#endif
  END_REGIST_PLAYUVER_SUPPORTED_FMT;
}

std::vector<PlaYUVerStdResolution> PlaYUVerStream::stdResolutionSizes()
{

#define REGIST_PLAYUVER_STANDARD_RESOLUTION( name, width, height) \
    stdResElement.shortName = name; \
    stdResElement.uiWidth = width; \
    stdResElement.uiHeight = height; \
    stdResList.push_back( stdResElement );

  std::vector<PlaYUVerStdResolution> stdResList;
  PlaYUVerStdResolution stdResElement;
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "CIF", 352, 288 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "VGA", 640, 480 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "WVGA", 832, 480 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "XVGA", 1024, 768 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "HD", 1280, 720 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "SXGA-", 1280, 900 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "SXGA", 1280, 1024 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "WSXGA", 1440, 900 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "FullHD", 1920, 1080 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "WQXGA", 2560, 1600 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "UltraHD", 3840, 2160 );
  REGIST_PLAYUVER_STANDARD_RESOLUTION( "8K", 8192, 4608 );
  return stdResList;
}

PlaYUVerStream::PlaYUVerStream()
{
  m_bInit = false;
  m_bIsInput = true;
  m_bIsOpened = false;
  m_bLoadAll = false;
  m_iStreamHandler = INVALID_HANDLER;
  m_pFile = NULL;
  m_pchFilename = NULL;
  m_uiWidth = 0;
  m_uiHeight = 0;
  m_uiTotalFrameNum = 0;
  m_iCurrFrameNum = -1;
  m_iPixelFormat = -1;
  m_uiBitsPerPixel = 8;
  m_dFrameRate = 30;
  m_pStreamBuffer = NULL;
  m_cFilename = "";
  m_pcCurrFrame = NULL;
  m_pcNextFrame = NULL;
  m_ppcFrameBuffer = NULL;
  m_uiFrameBufferSize = 2;
  m_uiFrameBufferIndex = 0;
  m_uiCurrFrameFileIdx = 0;
#ifdef USE_FFMPEG
  m_cLibAvContext = new LibAvContextHandle;
#endif
}

PlaYUVerStream::~PlaYUVerStream()
{
  close();
#ifdef USE_FFMPEG
  if( m_cLibAvContext )
    delete m_cLibAvContext;
#endif
}

Void PlaYUVerStream::findHandler()
{
  m_iStreamHandler = INVALID_HANDLER;

//  if( m_uiBitsPerPixel != 8 )
//  {
//    m_iStreamHandler = YUV_IO;
//    return;
//  }

  std::string currExt = m_cFilename.substr( m_cFilename.find_last_of( "." ) + 1 );
  currExt = lowercase( currExt );

  if( m_bIsInput )
  {
#ifdef USE_FFMPEG
    std::vector<PlaYUVerSupportedFormat> supportedFmts = LibAvContextHandle::supportedReadFormats();
    for( UInt i = 0; i < supportedFmts.size(); i++ )
    {
      if( supportedFmts[i].formatExt == currExt )
      {
        m_cFormatName = supportedFmts[i].formatExt[i];
        m_iStreamHandler = FFMPEG;
        return;
      }
    }
#endif
    if( m_iStreamHandler == INVALID_HANDLER )
      m_iStreamHandler = YUV_IO;
  }
  else
  {
#ifdef USE_FFMPEG
    std::vector<PlaYUVerSupportedFormat> supportedFmts = LibAvContextHandle::supportedWriteFormats();
    for( UInt i = 0; i < supportedFmts.size(); i++ )
    {
      if( supportedFmts[i].formatExt == currExt )
      {
        m_cFormatName = supportedFmts[i].formatExt;
        m_iStreamHandler = FFMPEG;
        return;
      }
    }
#endif
    if( currExt == "yuv" )
    {
      m_iStreamHandler = YUV_IO;
    }
  }
}

Bool PlaYUVerStream::open( std::string filename, std::string resolution, std::string input_format_name, UInt bitsPel, UInt frame_rate, Bool bInput )
{
  UInt width;
  UInt height;
  Int input_format = -1;

  if( resolution.size() > 0 )
  {
    sscanf( resolution.c_str(), "%ux%u", &width, &height );
    if( width <= 0 || height <= 0 )
    {
      return false;
    }
  }
  for( UInt i = 0; i < PlaYUVerFrame::supportedPixelFormatListNames().size(); i++ )
  {
    if( lowercase( PlaYUVerFrame::supportedPixelFormatListNames()[i] ) == lowercase( input_format_name ) )
    {
      input_format = i;
      break;
    }
  }
  return open( filename, width, height, input_format, bitsPel, frame_rate, bInput );
}

Bool PlaYUVerStream::open( std::string filename, UInt width, UInt height, Int input_format, UInt bitsPel, UInt frame_rate, Bool bInput )
{
  if( m_bInit )
  {
    close();
  }
  m_bInit = false;
  m_bIsInput = bInput;
  m_cFilename = filename;
  m_uiWidth = width;
  m_uiHeight = height;
  m_iPixelFormat = input_format;
  m_uiBitsPerPixel = bitsPel;
  m_dFrameRate = frame_rate;

  m_pchFilename = new Char[m_cFilename.size() + 1];
  std::copy( m_cFilename.begin(), m_cFilename.end(), m_pchFilename );
  m_pchFilename[m_cFilename.size()] = '\0';  // don't forget the terminating 0

  findHandler();
  if( m_iStreamHandler == INVALID_HANDLER )
  {
    close();
    throw "Invalid file format";
    return m_bInit;
  }

  m_uiFrameBufferSize = m_bIsInput ? 2 : 1;

#ifdef USE_FFMPEG
  if( m_iStreamHandler == FFMPEG )
  {
    if( !m_cLibAvContext->initAvFormat( m_pchFilename, m_uiWidth, m_uiHeight, m_iPixelFormat, m_uiBitsPerPixel, m_dFrameRate, m_uiTotalFrameNum ) )
    {
      throw "Cannot open file using FFmpeg libs";
    }
    m_cCodedName = m_cLibAvContext->getCodecName();
  }
#endif

#ifdef USE_OPENCV
  if( m_iStreamHandler == OPENCV_HANDLER )
  {
    m_pcCurrFrame = LibOpenCVHandler::loadFrame( m_cFilename );
    if( m_pcCurrFrame )
    {
      m_uiWidth = m_pcCurrFrame->getWidth();
      m_uiHeight = m_pcCurrFrame->getHeight();
      m_iPixelFormat = m_pcCurrFrame->getPelFormat();
    }
  }
#endif

  if( m_uiWidth <= 0 || m_uiHeight <= 0 || m_iPixelFormat < 0 )
  {
    close();
    throw "[PlaYUVerStream] Incorrect configuration";
    return m_bInit;
  }

  if( m_iStreamHandler != OPENCV_HANDLER )
  {
    if( m_iStreamHandler == YUV_IO )
    {
      if( !openFile() )
      {
        close();
        throw "[PlaYUVerStream] Cannot open file";
        return m_bInit;
      }
      m_cFormatName = "YUV";
      m_cCodedName = "Raw Video";
    }

    getMem1D<PlaYUVerFrame*>( &m_ppcFrameBuffer, m_uiFrameBufferSize );
    for( UInt i = 0; i < m_uiFrameBufferSize; i++ )
    {
      try
      {
        m_ppcFrameBuffer[i] = new PlaYUVerFrame( m_uiWidth, m_uiHeight, m_iPixelFormat, m_uiBitsPerPixel );
      }
      catch( const char *msg )
      {
        close();
        throw "[PlaYUVerStream] Cannot allocated frame buffer";
        return m_bInit;
      }
    }
    m_uiFrameBufferIndex = 0;
    m_pcCurrFrame = m_ppcFrameBuffer[m_uiFrameBufferIndex];

    UInt64 frame_bytes_input = m_pcCurrFrame->getBytesPerFrame();

    if( m_iStreamHandler == YUV_IO && m_bIsInput )
    {
      fseek( m_pFile, 0, SEEK_END );
      UInt64 fileSize = ftell( m_pFile );
#if 0
      if( fileSize % frame_bytes_input )
      {
        Int remaining = fileSize % frame_bytes_input;
        close();
        throw "[PlaYUVerStream] Incorrect configuration: failed ( fileSize % frame_bytes_input = " << remaining << " )";
        return m_bInit;
      }
#endif
      m_uiTotalFrameNum = fileSize / frame_bytes_input;
      fseek( m_pFile, 0, SEEK_SET );
    }
  }
  else
  {
    m_pcNextFrame = NULL;
    m_uiFrameBufferSize = 0;
    m_uiFrameBufferIndex = 0;
    m_uiTotalFrameNum = 1;
  }

  if( m_bIsInput && m_uiTotalFrameNum <= 0 )
  {
    close();
    throw "[PlaYUVerStream] Incorrect configuration: less than one frame";
    return m_bInit;
  }

  if( !getMem1D<Byte>( &m_pStreamBuffer, m_pcCurrFrame->getBytesPerFrame() ) )
  {
    close();
    throw "[PlaYUVerStream] Cannot allocated memory";
    return m_bInit;
  }

  if( m_pcCurrFrame )
  {
    m_uiWidth = m_pcCurrFrame->getWidth();
    m_uiHeight = m_pcCurrFrame->getHeight();
    m_iPixelFormat = m_pcCurrFrame->getPelFormat();
    m_uiBitsPerPixel = m_pcCurrFrame->getBitsPel();
  }

  m_iCurrFrameNum = -1;
  m_bInit = true;

  seekInput( 0 );

  m_bInit = true;
  return m_bInit;
}

Bool PlaYUVerStream::reload()
{
#ifdef USE_FFMPEG
  if( m_iStreamHandler == FFMPEG )
  {
    m_cLibAvContext->closeAvFormat();
    if( !m_cLibAvContext->initAvFormat( m_pchFilename, m_uiWidth, m_uiHeight, m_iPixelFormat, m_uiBitsPerPixel, m_dFrameRate, m_uiTotalFrameNum ) )
    {
      throw "Cannot open file using FFmpeg libs";
    }
    m_cCodedName = m_cLibAvContext->getCodecName();
  }
#endif
  if( m_iStreamHandler == YUV_IO )
  {
    closeFile();
    if( !openFile() )
    {
      return false;
    }
    UInt64 frameBytesInput = m_pcCurrFrame->getBytesPerFrame();
    fseek( m_pFile, 0, SEEK_END );
    UInt64 fileSize = ftell( m_pFile );
    m_uiTotalFrameNum = fileSize / frameBytesInput;
    fseek( m_pFile, 0, SEEK_SET );
  }
  if( m_uiWidth <= 0 || m_uiHeight <= 0 || m_iPixelFormat < 0 || m_uiTotalFrameNum < 1 )
  {
    return false;
  }
  if( UInt( m_iCurrFrameNum ) >= m_uiTotalFrameNum )
  {
    m_iCurrFrameNum = 0;
  }
  Int currFrameNum = m_iCurrFrameNum;
  m_iCurrFrameNum = -1;
  seekInput( currFrameNum );
  return true;
}

Bool PlaYUVerStream::openFile()
{
//m_fsIOStream.open( m_cFilename,  std::ios::in | std::ios::binary )
  m_pFile = NULL;
  m_pFile = fopen( m_pchFilename, m_bIsInput ? "rb" : "wb" );
  if( m_pFile == NULL )
  {
    return false;
  }
  if( m_bIsInput )
  {
    fseek( m_pFile, m_iCurrFrameNum >= 0 ? m_iCurrFrameNum : 0, SEEK_SET );
  }
  return true;
}

Void PlaYUVerStream::closeFile()
{
  if( m_pFile )
    fclose( m_pFile );
}

Void PlaYUVerStream::close()
{
  if( !m_bInit )
    return;

  if( m_iStreamHandler == YUV_IO )
    closeFile();

#ifdef USE_FFMPEG
  if( m_iStreamHandler == FFMPEG )
    m_cLibAvContext->closeAvFormat();
#endif

  if( m_ppcFrameBuffer )
  {
    for( UInt i = 0; i < m_uiFrameBufferSize; i++ )
    {
      if( m_ppcFrameBuffer[i] )
        delete m_ppcFrameBuffer[i];
      m_ppcFrameBuffer[i] = NULL;
    }
    freeMem1D<PlaYUVerFrame*>( m_ppcFrameBuffer );
    m_ppcFrameBuffer = NULL;
  }

  if( m_pStreamBuffer )
    freeMem1D<Byte>( m_pStreamBuffer );

  if( m_pchFilename )
    delete[] m_pchFilename;

  m_bLoadAll = false;
  m_bInit = false;
}

Void PlaYUVerStream::getFormat( UInt& rWidth, UInt& rHeight, Int& rInputFormat, UInt& rBitsPerPel, UInt& rFrameRate )
{
  if( m_bInit )
  {
    rWidth = m_uiWidth;
    rHeight = m_uiHeight;
    rInputFormat = m_iPixelFormat;
    rBitsPerPel = m_uiBitsPerPixel;
    rFrameRate = m_dFrameRate;
  }
  else
  {
    rWidth = 0;
    rHeight = 0;
    rInputFormat = PlaYUVerFrame::YUV420p;
    rBitsPerPel = 8;
    rFrameRate = 30;
  }
}

Void PlaYUVerStream::loadAll()
{
  if( m_bLoadAll || !m_bIsInput )
    return;

  if( m_ppcFrameBuffer )
  {
    for( UInt i = 0; i < m_uiFrameBufferSize; i++ )
    {
      if( m_ppcFrameBuffer[i] )
        delete m_ppcFrameBuffer[i];
      m_ppcFrameBuffer[i] = NULL;
    }
    freeMem1D<PlaYUVerFrame*>( m_ppcFrameBuffer );
    m_ppcFrameBuffer = NULL;
  }
  m_uiFrameBufferSize = m_uiTotalFrameNum;
  getMem1D<PlaYUVerFrame*>( &m_ppcFrameBuffer, m_uiFrameBufferSize );
  for( UInt i = 0; i < m_uiFrameBufferSize; i++ )
  {
    m_ppcFrameBuffer[i] = new PlaYUVerFrame( m_uiWidth, m_uiHeight, m_iPixelFormat, m_uiBitsPerPixel );
    if( !m_ppcFrameBuffer[i] )
    {
      close();
    }
  }
  m_uiFrameBufferIndex = 0;
  m_pcCurrFrame = m_ppcFrameBuffer[m_uiFrameBufferIndex];
  m_iCurrFrameNum = -1;
  seekInput( 0 );
  for( UInt i = m_uiFrameBufferIndex + 1; i < m_uiFrameBufferSize; i++ )
  {
    m_pcNextFrame = m_ppcFrameBuffer[i];
    readFrame();
  }
  m_bLoadAll = true;
  m_uiFrameBufferIndex = 0;
  m_pcNextFrame = m_ppcFrameBuffer[m_uiFrameBufferIndex];
  m_iCurrFrameNum = -1;
  setNextFrame();
}

Void PlaYUVerStream::getDuration( Int* duration_array )
{
  Int hours, mins, secs = 0;
#ifdef USE_FFMPEG
  if( m_iStreamHandler == FFMPEG )
  {
    secs = m_cLibAvContext->getStreamDuration();
  }
  else
#endif
  if( m_iStreamHandler == YUV_IO )
  {
    secs = m_uiTotalFrameNum / m_dFrameRate;
  }
  mins = secs / 60;
  secs %= 60;
  hours = mins / 60;
  mins %= 60;
  *duration_array++ = hours;
  *duration_array++ = mins;
  *duration_array++ = secs;
}

Void PlaYUVerStream::readFrameFillRGBBuffer()
{
  readFrame();
  if( m_pcNextFrame )
    m_pcNextFrame->fillRGBBuffer();
  return;
}

Void PlaYUVerStream::readFrame()
{
  if( !m_bInit || !m_bIsInput || m_bLoadAll )
    return;

  if( m_uiCurrFrameFileIdx >= m_uiTotalFrameNum )
  {
    m_pcNextFrame = NULL;
    return;
  }

  if( m_iStreamHandler == OPENCV_HANDLER )
  {
    return;
  }

#ifdef USE_FFMPEG
  if( m_iStreamHandler == FFMPEG )
  {
    if( !m_cLibAvContext->decodeAvFormat() )
    {
      throw "[PlaYUVerStream] Error reading frame from libav";
      return;
    }
    m_pcNextFrame->frameFromBuffer( m_cLibAvContext->m_pchFrameBuffer, m_cLibAvContext->m_uiFrameBufferSize );
  }
#endif
  if( m_iStreamHandler == YUV_IO )
  {
    UInt64 frame_bytes_input = m_pcNextFrame->getBytesPerFrame();
    UInt64 bytes_read = fread( m_pStreamBuffer, sizeof(Byte), frame_bytes_input, m_pFile );
    if( bytes_read != frame_bytes_input )
    {
      throw "[PlaYUVerStream] Cannot read file";
      return;
    }
    m_pcNextFrame->frameFromBuffer( m_pStreamBuffer, bytes_read );
  }
  m_uiCurrFrameFileIdx++;
  return;
}

Void PlaYUVerStream::writeFrame()
{
  writeFrame( m_pcCurrFrame );
}

Void PlaYUVerStream::writeFrame( PlaYUVerFrame *pcFrame )
{
  if( m_iStreamHandler == YUV_IO )
  {
    UInt64 frame_bytes_input = pcFrame->getBytesPerFrame();
    pcFrame->frameToBuffer( m_pStreamBuffer );
    UInt64 bytes_read = fwrite( m_pStreamBuffer, sizeof(Byte), frame_bytes_input, m_pFile );
    if( bytes_read != frame_bytes_input )
    {
      throw "[PlaYUVerStream] Cannot write into the file";
    }
  }
  return;
}

Bool PlaYUVerStream::saveFrame( const std::string& filename )
{
  return saveFrame( filename, m_pcCurrFrame );
}

Bool PlaYUVerStream::saveFrame( const std::string& filename, PlaYUVerFrame *saveFrame )
{
  Int iFileFormat = INVALID_HANDLER;
  std::string fmtExt;

//std::string currExt = QFileInfo( qtString ).suffix().toStdString();
  std::string currExt = filename.substr( filename.find_last_of( "." ) + 1 );

  std::vector<PlaYUVerSupportedFormat> supportedFmts = PlaYUVerStream::supportedSaveFormats();
  for( UInt i = 0; i < supportedFmts.size(); i++ )
  {
    if( supportedFmts[i].formatExt == currExt )
    {
      iFileFormat = i;
      fmtExt = supportedFmts[i].formatExt[i];
      break;
    }
  }

  if( iFileFormat != INVALID_HANDLER )
  {
    if( fmtExt == "yuv" )
    {
      PlaYUVerStream auxFrameStream;
      if( !auxFrameStream.open( filename, saveFrame->getWidth(), saveFrame->getHeight(), saveFrame->getPelFormat(), saveFrame->getBitsPel(), 1, false ) )
      {
        return false;
      }
      auxFrameStream.writeFrame( saveFrame );
      auxFrameStream.close();
      return true;
    }
#if 0
    else
    {
      saveFrame->fillRGBBuffer();
      QImage qimg = QImage( saveFrame->getRGBBuffer(), saveFrame->getWidth(), saveFrame->getHeight(), QImage::Format_RGB32 );
      return qimg.save( qtString );
    }
#endif
#ifdef USE_OPENCV
    else
    {
      return LibOpenCVHandler::saveFrame( saveFrame, filename );
    }
#endif
  }
  return false;
}

Bool PlaYUVerStream::setNextFrame()
{
  Bool bEndOfSeq = false;
  if( m_pcNextFrame )
  {
    m_pcCurrFrame = m_pcNextFrame;
    m_iCurrFrameNum++;
    if( m_iCurrFrameNum + 1 < Int( m_uiTotalFrameNum ) )
    {
      m_uiFrameBufferIndex++;
      if( m_uiFrameBufferIndex == m_uiFrameBufferSize )
      {
        m_uiFrameBufferIndex = 0;
      }
      m_pcNextFrame = m_ppcFrameBuffer[m_uiFrameBufferIndex];
    }
    else
    {
      m_pcNextFrame = NULL;
    }
  }
  else
  {
    bEndOfSeq = true;
  }
  return bEndOfSeq;
}

PlaYUVerFrame* PlaYUVerStream::getCurrFrame( PlaYUVerFrame *pyuv_image )
{
  if( pyuv_image == NULL )
    pyuv_image = new PlaYUVerFrame( m_pcCurrFrame );
  else
    pyuv_image->copyFrom( m_pcCurrFrame );
  return pyuv_image;
}

PlaYUVerFrame* PlaYUVerStream::getCurrFrame()
{
  return m_pcCurrFrame;
}

PlaYUVerFrame* PlaYUVerStream::getNextFrame()
{
  return m_pcNextFrame;
}

Bool PlaYUVerStream::seekInputRelative( Bool bIsFoward )
{
  if( !m_bInit || !m_bIsInput )
    return false;

  Bool bRet = false;
  if( bIsFoward )
  {
    bRet = !setNextFrame();
    readFrame();
  }
  else
  {
    UInt64 newFrameNum = m_iCurrFrameNum - 1;
    bRet = seekInput( newFrameNum );
  }
  return bRet;
}

Bool PlaYUVerStream::seekInput( UInt64 new_frame_num )
{
  if( !m_bInit || new_frame_num < 0 || new_frame_num >= m_uiTotalFrameNum || ( Int64 )new_frame_num == m_iCurrFrameNum )
    return false;

  m_iCurrFrameNum = new_frame_num - 1;

  if( m_bLoadAll )
  {
    m_uiFrameBufferIndex = new_frame_num;
    m_pcNextFrame = m_ppcFrameBuffer[m_uiFrameBufferIndex];
    setNextFrame();
    return true;
  }
  if( m_iStreamHandler == OPENCV_HANDLER )
  {
    return true;
  }
#ifdef USE_FFMPEG
  if( m_iStreamHandler == FFMPEG )
  {
    m_cLibAvContext->seekAvFormat( new_frame_num );
  }
#endif
  if( m_iStreamHandler == YUV_IO )
  {
    if( !m_pFile )
      return false;
    UInt64 frame_bytes_input = m_ppcFrameBuffer[0]->getBytesPerFrame();
    UInt64 nbytes_seek = frame_bytes_input * new_frame_num;
    fseek( m_pFile, nbytes_seek, SEEK_SET );
  }
  m_uiCurrFrameFileIdx = new_frame_num;

  m_uiFrameBufferIndex = 0;
  m_pcCurrFrame = m_pcNextFrame = m_ppcFrameBuffer[m_uiFrameBufferIndex];

  readFrame();
  setNextFrame();
  if( m_uiTotalFrameNum > 1 )
    readFrame();
  return true;
}

}  // NAMESPACE
