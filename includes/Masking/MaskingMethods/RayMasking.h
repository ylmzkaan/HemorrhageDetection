#pragma once
#include "../MaskingStrategy.h"
//#include "../../Helper.h"
#include <itkBinaryThresholdImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkExtractImageFilter.h>
#include <itkPathConstIterator.h>
#include "itkCastImageFilter.h"
#include "itkTileImageFilter.h"
#include <itkChainCodePath2D.h>
#include <itkImage.h>
#include <SimpleITK.h>
#include <iostream>
#include <math.h>

namespace sitk = itk::simple;
using namespace std;

using InputPixelType = int;
using MaskPixelType = uint8_t;
using InputImageType = itk::Image< InputPixelType,  3 >;
using MaskImage2DType = itk::Image< MaskPixelType, 2 >;
using MaskImageType = itk::Image< MaskPixelType, 3 >;

using FilterType = itk::BinaryThresholdImageFilter< InputImageType, MaskImageType >;
using ExtractFilterType = itk::ExtractImageFilter< MaskImageType, MaskImage2DType >;
using TileImageFilterType = itk::TileImageFilter< MaskImage2DType, MaskImageType >;

using iterType = itk::ImageRegionIterator<MaskImage2DType>;
using constIterType = itk::ImageRegionConstIterator<MaskImage2DType>;
using chainCodeType = itk::ChainCodePath2D;
using pathConstIterType = itk::PathConstIterator< MaskImage2DType, chainCodeType >;

class RayMasking : public MaskingStrategy
{
    private:

    const InputPixelType lowerThreshold = -20;
    const InputPixelType upperThreshold = 90;
    const InputPixelType boneLowerThreshold = 300;
    const InputPixelType boneUpperThreshold = 2000;

    sitk::CastImageFilter               caster;
    sitk::BinaryThresholdImageFilter    binaryThresholdImageFilter;
    TileImageFilterType::Pointer        tileFilter = TileImageFilterType::New();

    public:

    RayMasking(sitk::Image &image);
    void run();

    private:

    sitk::Image isolateIntracranialVoxels(sitk::Image &initialMask, sitk::Image &boneMask);
    
};