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
 * \file     SaliencyDetection.cpp
 * \brief    Modules to measure optical flow
 */

#include "SaliencyDetection.h"

#include <cstdlib>

//#include <opencv2/video/video.hpp>

using cv::Mat;
using cv::Ptr;
using namespace cv::saliency;

SaliencyDetectionModule::SaliencyDetectionModule()
{
  /* Module Definition */
  m_iModuleAPI = CLP_MODULE_API_2;
  m_iModuleType = CLP_FRAME_PROCESSING_MODULE;
  m_pchModuleCategory = "Saliency";
  m_uiNumberOfFrames = 1;
  m_uiModuleRequirements = CLP_MODULE_REQUIRES_NEW_WINDOW;

  m_pcSaliencyFrame = NULL;
}

bool SaliencyDetectionModule::commonCreate( std::vector<CalypFrame*> apcFrameList )
{
  _BASIC_MODULE_API_2_CHECK_

  m_pcSaliencyFrame = new CalypFrame( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight(), CLP_GRAY );

  return true;
}

bool SaliencyDetectionModule::commonProcess( std::vector<CalypFrame*> apcFrameList )
{
  Mat cvFrame;
  if( !apcFrameList[0]->toMat( cvFrame, true ) )
  {
    return m_pcSaliencyFrame;
  }
  cvFrame.copyTo( m_matSaliency );
  return m_ptrSaliencyAlgorithm->computeSaliency( cvFrame, m_matSaliency );
}

void SaliencyDetectionModule::destroy()
{
  if( m_pcSaliencyFrame )
    delete m_pcSaliencyFrame;
}

SaliencyDetectionSpectral::SaliencyDetectionSpectral()
{
  /* Module Definition */
  m_pchModuleName = "SaliencySpectralResidual";
  m_pchModuleLongName = "Spectral residual";
  m_pchModuleTooltip = "Measure saliency using spectral residual method";

  m_uiModuleRequirements = m_uiModuleRequirements | CLP_MODULE_REQUIRES_OPTIONS;

  m_cModuleOptions.addOptions() /**/
      ( "Binary map", m_bBinaryMap, "Measure a binary saliency map [false]" );

  m_bBinaryMap = false;
}

bool SaliencyDetectionSpectral::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;

  m_ptrSaliencyAlgorithm = StaticSaliencySpectralResidual::create();

  return true;
}

CalypFrame* SaliencyDetectionSpectral::process( std::vector<CalypFrame*> apcFrameList )
{
  if( commonProcess( apcFrameList ) )
  {
    Mat matBinaryMap;
    StaticSaliencySpectralResidual spec;
    spec.computeBinaryMap( m_matSaliency, matBinaryMap );
    if( m_bBinaryMap )
    {
      m_pcSaliencyFrame->fromMat( matBinaryMap );
    }
    else
    {
      m_matSaliency *= 255;
      m_matSaliency.convertTo( m_matSaliency, CV_8UC1 );
      m_pcSaliencyFrame->fromMat( m_matSaliency );
    }
  }
  return m_pcSaliencyFrame;
}

SaliencyDetectionFineGrained::SaliencyDetectionFineGrained()
{
  /* Module Definition */
  m_pchModuleName = "SaliencyDetectionFineGrained";
  m_pchModuleLongName = "Fine grained";
  m_pchModuleTooltip = "Measure saliency using fine grained method";
}

bool SaliencyDetectionFineGrained::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;
  m_ptrSaliencyAlgorithm = cv::saliency::StaticSaliencyFineGrained::create();
  return true;
}

CalypFrame* SaliencyDetectionFineGrained::process( std::vector<CalypFrame*> apcFrameList )
{
  if( commonProcess( apcFrameList ) )
    m_pcSaliencyFrame->fromMat( m_matSaliency );
  return m_pcSaliencyFrame;
}

SaliencyDetectionBinWangApr2014::SaliencyDetectionBinWangApr2014()
{
  /* Module Definition */
  m_pchModuleName = "SaliencyDetectionBinWangApr2014";
  m_pchModuleLongName = "Fast Self-tuning Background Subtraction Algorithm";
  m_pchModuleTooltip = "Measure saliency using a fast self-tuning background subtraction algorithm";
}

bool SaliencyDetectionBinWangApr2014::create( std::vector<CalypFrame*> apcFrameList )
{
  bool bRet = commonCreate( apcFrameList );
  if( !bRet )
    return bRet;
  m_ptrSaliencyAlgorithm = MotionSaliencyBinWangApr2014::create();
  m_ptrSaliencyAlgorithm.dynamicCast<MotionSaliencyBinWangApr2014>()->setImagesize( apcFrameList[0]->getWidth(), apcFrameList[0]->getHeight() );
  m_ptrSaliencyAlgorithm.dynamicCast<MotionSaliencyBinWangApr2014>()->init();
  return true;
}

CalypFrame* SaliencyDetectionBinWangApr2014::process( std::vector<CalypFrame*> apcFrameList )
{
  if( commonProcess( apcFrameList ) )
  {
    m_matSaliency *= 255;
    m_matSaliency.convertTo( m_matSaliency, CV_8UC1 );
    //cv::cvtColor(m_matSaliency, m_matSaliency, cv::COLOR_BGR2GRAY);
    m_pcSaliencyFrame->fromMat( m_matSaliency );
  }
  return m_pcSaliencyFrame;
}
