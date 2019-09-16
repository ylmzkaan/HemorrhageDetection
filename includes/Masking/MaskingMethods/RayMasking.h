#pragma once
#include "../MaskingStrategy.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include <sitkDiscreteGaussianImageFilter.h>
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

using InputPixelType = int;
using MaskPixelType = uint8_t;
using InputImageType = itk::Image< InputPixelType,  3 >;
using InputImage2DType = itk::Image< InputPixelType,  2 >;
using MaskImage2DType = itk::Image< MaskPixelType, 2 >;
using MaskImageType = itk::Image< MaskPixelType, 3 >;

using FilterType = itk::BinaryThresholdImageFilter< InputImageType, MaskImageType >;
using ExtractFilterType = itk::ExtractImageFilter< MaskImageType, MaskImage2DType >;
using TileMaskFilterType = itk::TileImageFilter< MaskImage2DType, MaskImageType >;
using TileImageFilterType = itk::TileImageFilter< InputImage2DType, InputImageType >;
using ConnectedComponentImageFilterType = itk::ConnectedComponentImageFilter < MaskImage2DType, InputImage2DType >;
using LabelShapeKeepNObjectsImageFilterType = itk::LabelShapeKeepNObjectsImageFilter< InputImage2DType >;

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

    sitk::CastImageFilter                           caster;
    sitk::BinaryThresholdImageFilter                binaryThresholdImageFilter;
    sitk::DiscreteGaussianImageFilter               gaussianFilter;

    TileMaskFilterType::Pointer                     tileMaskFilter = TileMaskFilterType::New();
    TileImageFilterType::Pointer                    tileFilter = TileImageFilterType::New();
    ConnectedComponentImageFilterType::Pointer      connCompFilter = ConnectedComponentImageFilterType::New();
    LabelShapeKeepNObjectsImageFilterType::Pointer  labelShapeKeepNObjectsImageFilter = LabelShapeKeepNObjectsImageFilterType::New();

    public:

    RayMasking(sitk::Image &image);
    void run();

    private:

    sitk::Image isolateIntracranialVoxels(sitk::Image &initialMask, sitk::Image &boneMask);
    sitk::Image LCC( sitk::Image inputMask, int axis);
    void sitkToBinaryItk( const sitk::Image &image, MaskImageType::Pointer &outputImage );    
};