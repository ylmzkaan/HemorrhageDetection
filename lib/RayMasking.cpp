#include "../includes/Masking/MaskingMethods/RayMasking.h"

RayMasking::RayMasking(sitk::Image &image)
: MaskingStrategy(image) {}

void RayMasking::run()
{
    //image = gaussianFilter.Execute(image);

    sitk::Image initialMask = binaryThresholdImageFilter.Execute(image, lowerThreshold, upperThreshold, 1, 0);
    sitk::Image boneMask = binaryThresholdImageFilter.Execute(image, boneLowerThreshold, boneUpperThreshold, 1, 0);
    mask = isolateIntracranialVoxels(initialMask, boneMask);
    mask = LCC(mask, 2); // along z-axis
}

void RayMasking::sitkToBinaryItk( const sitk::Image &image, MaskImageType::Pointer &outputImage )
{
    caster.SetOutputPixelType( sitk::sitkUInt8 );
    sitk::Image outputMaskInt = caster.Execute( image );

    outputImage = dynamic_cast <MaskImageType*>( outputMaskInt.GetITKBase() );
    outputImage->DisconnectPipeline();
    outputImage->SetBufferedRegion( outputImage->GetLargestPossibleRegion() );
}

MaskImageType::RegionType initSliceGenerator( const MaskImageType::Pointer &image, const MaskImageType::SizeType &size )
{
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetDirectionCollapseToSubmatrix();
    extractFilter->SetInput( image );
    MaskImageType::RegionType desiredRegion;
    desiredRegion.SetSize(  size  );
    return desiredRegion;
}

sitk::Image RayMasking::isolateIntracranialVoxels(sitk::Image &initialMask, sitk::Image &boneMask)
{
    // Cast inputs to itk images
    MaskImageType::Pointer outputMask = MaskImageType::New();
    sitkToBinaryItk( initialMask, outputMask );
    outputMask->FillBuffer(0);

    MaskImageType::Pointer itkInitialMask = MaskImageType::New();
    sitkToBinaryItk( initialMask, itkInitialMask );

    MaskImageType::Pointer itkBoneMask = MaskImageType::New();
    sitkToBinaryItk( boneMask, itkBoneMask );

    // Extract filter for initial mask
    MaskImageType::IndexType start = { 0,0,0 };
    MaskImageType::RegionType inputRegion = itkInitialMask->GetLargestPossibleRegion();
    MaskImageType::SizeType size = inputRegion.GetSize();
    size[2] = 0; // extract along z direction

    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetDirectionCollapseToSubmatrix();
    extractFilter->SetInput( itkInitialMask );
    MaskImageType::RegionType desiredRegion;
    desiredRegion.SetSize(  size  );

    // Extract filter for bone mask
    ExtractFilterType::Pointer extractFilterBone = ExtractFilterType::New();
    extractFilterBone->SetDirectionCollapseToSubmatrix();
    extractFilterBone->SetInput( itkBoneMask );
    MaskImageType::RegionType desiredRegionBone;
    desiredRegionBone.SetSize(  size  );

    // Extract filter for output
    ExtractFilterType::Pointer extractFilterOutput = ExtractFilterType::New();
    extractFilterOutput->SetDirectionCollapseToSubmatrix();
    extractFilterOutput->SetInput( outputMask );
    MaskImageType::RegionType desiredRegionOutput;
    desiredRegionOutput.SetSize(  size  );
    
    int nSlices = inputRegion.GetSize()[2];
    constexpr unsigned int intersectionTreshold = 6;

    itk::FixedArray< unsigned int, 3 > layout;
    layout[0] = 1;
    layout[1] = 1;
    layout[2] = nSlices;

    tileMaskFilter->SetLayout( layout );

    MaskImage2DType::Pointer initialMaskSlice;
    MaskImage2DType::Pointer boneMaskSlice;
    MaskImage2DType::Pointer outputMaskSlice;

    for (int sliceIdx = 0; sliceIdx < nSlices; ++sliceIdx)
    {
        std::cout << "Slice idx: " << sliceIdx << std::endl;

        // Extract 2D slice from 3D image
        start[2] = sliceIdx;
        
        desiredRegion.SetIndex( start );
        extractFilter->SetExtractionRegion( desiredRegion );
        extractFilter->Update();
        initialMaskSlice = extractFilter->GetOutput();

        desiredRegionBone.SetIndex( start );
        extractFilterBone->SetExtractionRegion( desiredRegionBone );
        extractFilterBone->Update();
        boneMaskSlice = extractFilterBone->GetOutput();

        desiredRegionOutput.SetIndex( start );
        extractFilterOutput->SetExtractionRegion( desiredRegionOutput );
        extractFilterOutput->Update();
        outputMaskSlice = extractFilterOutput->GetOutput();

        // Ray casting
        // Init iterators
        constIterType it(initialMaskSlice, initialMaskSlice->GetLargestPossibleRegion());
        iterType itOutput(outputMaskSlice, outputMaskSlice->GetLargestPossibleRegion());

        chainCodeType::Pointer chainCode = chainCodeType::New();
        pathConstIterType pathIter(boneMaskSlice, chainCode);

        MaskImage2DType::IndexType currentIndex;
        unsigned int numberOfIntersections;
        std::vector<long> numberOfSteps;

        // Ray directions in freeman codes
        std::vector<unsigned int> pathDirections = { 1,2,3,4,5,6,7,8 };

        // Iterate over image
        it.GoToBegin();
        itOutput.GoToBegin();
        while ( !it.IsAtEnd() )
        {
            if (it.Get() == 0) { ++it; ++itOutput; continue; }
            
            numberOfIntersections = 0;
            currentIndex = it.GetIndex();
            numberOfSteps = {
                                511 - currentIndex[1], 
                                std::min( 511 - currentIndex[0], 511 - currentIndex[1]), 
                                511 - currentIndex[0], 
                                std::min( 511 - currentIndex[0], currentIndex[1]), 
                                currentIndex[1], 
                                std::min( currentIndex[0], currentIndex[1]),
                                currentIndex[0],
                                std::min( currentIndex[0], 511 - currentIndex[1])
                            };

            chainCode->SetStart( currentIndex );
            for (unsigned int i : pathDirections)
            {
                // Construct chain path
                chainCode->FillWithSteps(numberOfSteps[i-1], i);

                // Iterate image over the path
                pathIter.GoToBegin();
                while ( !pathIter.IsAtEnd() )
                {
                    if ( pathIter.Get() == 1)
                    {
                        ++numberOfIntersections;
                        break;
                    }
                    ++pathIter;
                }
                chainCode->Clear();
            }

            if (numberOfIntersections > intersectionTreshold) { itOutput.Set(1); }
            ++it; ++itOutput;
        }
        outputMaskSlice->DisconnectPipeline();
        tileMaskFilter->SetInput( sliceIdx, outputMaskSlice );
    }   

    MaskImageType::Pointer output = tileMaskFilter->GetOutput();
    sitk::Image outputImage = sitk::Image( output );
    return outputImage;
}



/*
Extracts largest connected components for each slice along an axis
 */
sitk::Image RayMasking::LCC( sitk::Image inputMask, int axis=2)
{
    MaskImageType::Pointer itkMask = MaskImageType::New();
    
    // Cast input to itk image
    caster.SetOutputPixelType( sitk::sitkUInt8 );
    inputMask = caster.Execute( inputMask );
    itkMask = dynamic_cast <itk::Image< uint8_t, 3 >*>( inputMask.GetITKBase() );
    itkMask->SetBufferedRegion( itkMask->GetLargestPossibleRegion() );

    // Extract filter for initial mask
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetDirectionCollapseToSubmatrix();
    extractFilter->SetInput( itkMask );
    MaskImageType::RegionType inputRegion = itkMask->GetLargestPossibleRegion();
    MaskImageType::SizeType size = inputRegion.GetSize();
    size[axis] = 0; // we extract along an axis
    MaskImageType::IndexType start = { 0,0,0 };
    MaskImageType::RegionType desiredRegion;
    desiredRegion.SetSize(  size  );

    int nSlices = inputRegion.GetSize()[axis];
    itk::FixedArray< unsigned int, 3 > layout;
    layout.Fill(1);
    layout[axis] = nSlices;

    tileFilter->SetLayout( layout );

    MaskImage2DType::Pointer maskSlice = MaskImage2DType::New();
    InputImage2DType::Pointer outputSlice = InputImage2DType::New();

    for ( int i=0; i<nSlices; ++i)
    {
        // Extract slices
        start[axis] = i;
        desiredRegion.SetIndex( start );
        extractFilter->SetExtractionRegion( desiredRegion );
        extractFilter->Update();
        maskSlice = extractFilter->GetOutput();

        connCompFilter->SetInput( maskSlice );
        connCompFilter->Update();

        labelShapeKeepNObjectsImageFilter->SetInput( connCompFilter->GetOutput() );
        labelShapeKeepNObjectsImageFilter->SetBackgroundValue( 0 );
        labelShapeKeepNObjectsImageFilter->SetNumberOfObjects( 1 );
        labelShapeKeepNObjectsImageFilter->SetAttribute( LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);
        outputSlice = labelShapeKeepNObjectsImageFilter->GetOutput();
        sitk::Show( sitk::Image( outputSlice ) );
        outputSlice->DisconnectPipeline();
        tileFilter->SetInput(i, outputSlice);
    }

    InputImageType::Pointer itkOutput = tileFilter->GetOutput();
    itkOutput->DisconnectPipeline();
    sitk::Image output = sitk::Image( itkOutput );
    sitk::Show(output);
    return output;
}