###
### CMakeLists for calyp modules component
###

OPTION( CALYP_THREADED_MODULES "Use threads for modules"  OFF )

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} )

set(Calyp_Mod_SRCS
  CalypModulesFactory.cpp
)

#
# Modules List
#
ADD_MODULE( FilterComponentLuma "FilterComponent" )
ADD_MODULE( FilterComponentChromaU "FilterComponent" )
ADD_MODULE( FilterComponentChromaV "FilterComponent" )
ADD_MODULE( FrameDifference "FrameDifference" )
ADD_MODULE( FrameMask "FrameMask" )
ADD_MODULE( FrameRotate "FrameRotate" )
ADD_MODULE( AbsoluteFrameDifference "AbsoluteFrameDifference" )
ADD_MODULE( InterFrameVariance "InterFrameVariance" )
ADD_MODULE( FrameBinarization "FrameBinarization" )
ADD_MODULE( FrameShift "FrameShift" )
ADD_MODULE( FrameConcatenation "FrameConcatenation" )
ADD_MODULE( SetChromaHalfScale "SetChromaHalfScale" )
ADD_MODULE( EightBitsSampling "EightBitsSampling" )
ADD_MODULE( FrameCrop "FrameCrop" )
ADD_MODULE( LumaAverage "LumaAverage" )
ADD_MODULE( WeightedPSNR "WeightedPSNR" )
ADD_MODULE( HEVCIntraPrediction "HEVCIntraPrediction" )
ADD_MODULE( ThreeSixtySpatialtoTemporal "ThreeSixtySpatialtoTemporal" )
ADD_MODULE( ThreeSixtyTemporaltoSpatial "ThreeSixtyTemporaltoSpatial" )


ADD_MODULE_USE_OPENCV( MeasureOpticalFlowDualTVL1 "MeasureOpticalFlowDualTVL1" VERSION 2.4.13.0 MODULES video )
ADD_MODULE_USE_OPENCV( DisparityStereoBM "DisparityStereoBM" VERSION 2.4.13.0 MODULES calib3d )
ADD_MODULE_USE_OPENCV( DisparityStereoSGBM "DisparityStereoSGBM" VERSION 2.4.13.0 MODULES calib3d )
ADD_MODULE_USE_OPENCV( OpticalFlowDualTVL1 "OpticalFlow" VERSION  3.1.0 MODULES optflow )
ADD_MODULE_USE_OPENCV( OpticalFlowSparseToDense "OpticalFlow" VERSION  3.1.0 MODULES optflow )
ADD_MODULE_USE_OPENCV( OpticalFlowFarneback "OpticalFlow" VERSION 3.1.0 MODULES video optflow )
ADD_MODULE_USE_OPENCV( OpticalDeepFlow "OpticalFlow" VERSION 3.1.0 MODULES video optflow )
ADD_MODULE_USE_OPENCV( SaliencyDetectionSpectral "SaliencyDetection" VERSION 3.3.0 MODULES saliency )
ADD_MODULE_USE_OPENCV( SaliencyDetectionFineGrained "SaliencyDetection" VERSION 3.3.0 MODULES saliency )
ADD_MODULE_USE_OPENCV( SaliencyDetectionBinWangApr2014 "SaliencyDetection" VERSION 3.3.0 MODULES saliency )


#
# Create header and auxiliar files
#
CREATE_MODULE_MACROS()
INCLUDE_DIRECTORIES( ${OpenCV_INCLUDE_DIRS} )
ADD_LIBRARY( CalypModules ${Calyp_Mod_SRCS} )
TARGET_LINK_LIBRARIES( CalypModules ${PROJECT_LIBRARY}  ${OpenCV_LIBRARIES} )
IF( USE_DYNLOAD )
  TARGET_LINK_LIBRARIES( CalypModules -ldl )
ENDIF()
