#include "../includes/Masking/MaskingMethods/RayMasking.h"

RayMasking::RayMasking(sitk::Image &image)
: MaskingStrategy(image) {}

void RayMasking::run()
{
    sitk::Image initialMask = binaryThresholdImageFilter.Execute(image, lowerThreshold, upperThreshold, 1, 0);
    sitk::Image boneMask = binaryThresholdImageFilter.Execute(image, boneLowerThreshold, boneUpperThreshold, 1, 0);

    sitk::Image cranialMask = isolateIntracranialVoxels(initialMask, boneMask);
    mask = cranialMask;

    sitk::Show( mask );
}

sitk::Image RayMasking::isolateIntracranialVoxels(sitk::Image &initialMask, sitk::Image &boneMask)
{
    MaskImageType::Pointer outputMask = MaskImageType::New();

    // Cast inputs to itk images
    caster.SetOutputPixelType( sitk::sitkUInt8 );
    sitk::Image outputMaskInt = caster.Execute( initialMask );
    outputMask = dynamic_cast <itk::Image< uint8_t, 3 >*>( outputMaskInt.GetITKBase() );
    outputMask->DisconnectPipeline();
    outputMask->FillBuffer(0);

    MaskImageType::Pointer itkInitialMask = MaskImageType::New();
    initialMask = caster.Execute( initialMask );
    itkInitialMask = dynamic_cast <itk::Image< uint8_t, 3 >*>( initialMask.GetITKBase() );
    itkInitialMask->DisconnectPipeline();

    MaskImageType::Pointer itkBoneMask = MaskImageType::New();
    boneMask = caster.Execute( boneMask );
    itkBoneMask = dynamic_cast <itk::Image< uint8_t, 3 >*>( boneMask.GetITKBase() );
    itkBoneMask->DisconnectPipeline();

    // Extract filter for initial mask
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetDirectionCollapseToSubmatrix();
    extractFilter->SetInput( itkInitialMask );
    MaskImageType::RegionType inputRegion = itkInitialMask->GetLargestPossibleRegion();
    MaskImageType::SizeType size = inputRegion.GetSize();
    size[2] = 0; // we extract along z direction
    MaskImageType::IndexType start = { 0,0,0 };
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

    itk::FixedArray< unsigned int, 3 > layout;
    layout[0] = 1;
    layout[1] = 1;
    layout[2] = 0;

    tileFilter->SetLayout( layout );

    for (int sliceIdx = 0; sliceIdx < nSlices; ++sliceIdx)
    {
        cout << "Slice idx: " << sliceIdx << endl;
        // Extract slices
        start[2] = sliceIdx;
        desiredRegion.SetIndex( start );
        extractFilter->SetExtractionRegion( desiredRegion );
        extractFilter->Update();
        MaskImage2DType::Pointer initialMaskSlice = extractFilter->GetOutput();

        desiredRegionBone.SetIndex( start );
        extractFilterBone->SetExtractionRegion( desiredRegionBone );
        extractFilterBone->Update();
        MaskImage2DType::Pointer boneMaskSlice = extractFilterBone->GetOutput();

        desiredRegionOutput.SetIndex( start );
        extractFilterOutput->SetExtractionRegion( desiredRegionOutput );
        extractFilterOutput->Update();
        MaskImage2DType::Pointer outputMaskSlice = extractFilterOutput->GetOutput();

        // Ray casting
        // Init iterators
        constIterType it(initialMaskSlice, initialMaskSlice->GetLargestPossibleRegion());
        constIterType itBone(boneMaskSlice, boneMaskSlice->GetLargestPossibleRegion());
        iterType itOutput(outputMaskSlice, outputMaskSlice->GetLargestPossibleRegion());

        chainCodeType::Pointer chainCode = chainCodeType::New();
        pathConstIterType pathIter(boneMaskSlice, chainCode);

        MaskImage2DType::IndexType currentIndex;
        int numberOfIntersections;
        vector<long> numberOfSteps;

        // Ray directions in freeman codes
        vector<unsigned int> pathDirections = { 1,2,3,4,5,6,7,8 };

        // Iterate over image
        it.GoToBegin();
        itOutput.GoToBegin();
        while ( !it.IsAtEnd() )
        {
            if (it.Get() == 0) { ++it; ++itOutput; continue; }
            
            numberOfIntersections = 0;
            currentIndex = it.GetIndex();
            numberOfSteps = {
                                currentIndex[1],
                                min( 511 - currentIndex[0], currentIndex[1]),
                                511 - currentIndex[0],
                                min( 511 - currentIndex[0], 511 - currentIndex[1]),
                                511 - currentIndex[1],
                                min( currentIndex[0], 511 - currentIndex[1]),
                                currentIndex[0],
                                min( currentIndex[0], currentIndex[1])
                            };

            chainCode->SetStart( currentIndex );
            for (unsigned int i : pathDirections)
            {
                // Construct chain path
                for (int s = 0; s < numberOfSteps[i-1]; ++s)
                    chainCode->InsertStep(s, i);

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

            if (numberOfIntersections > 6) { itOutput.Set(1); }
            ++it; ++itOutput;
        }
        tileFilter->SetInput( sliceIdx, outputMaskSlice );
    }   

    tileFilter->Update();
    MaskImageType::Pointer output = tileFilter->GetOutput();
    output->DisconnectPipeline();
    return sitk::Image( output );
}