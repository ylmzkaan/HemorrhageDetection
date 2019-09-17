#include "../includes/Masking/MaskingMethods/RayMasking.h"

RayMasking::RayMasking(sitk::Image &image)
: MaskingStrategy(image) {}

void RayMasking::run()
{
    //image = gaussianFilter.Execute(image);

    sitk::Image initialMask = binaryThresholdImageFilter.Execute(image, lowerThreshold, upperThreshold, 1, 0);
    sitk::Image boneMask = binaryThresholdImageFilter.Execute(image, boneLowerThreshold, boneUpperThreshold, 1, 0);

    mask = isolateIntracranialVoxels(initialMask, boneMask);
    //mask = LCC(mask, 2); // along z-axis
}

void RayMasking::sitkToBinaryItk( const sitk::Image &image, MaskImageType::Pointer &outputImage )
{
    caster.SetOutputPixelType( sitk::sitkUInt8 );
    sitk::Image castedImage = caster.Execute( image );

    outputImage = dynamic_cast <MaskImageType*>( castedImage.GetITKBase() );
    outputImage->DisconnectPipeline();
    outputImage->SetBufferedRegion( outputImage->GetLargestPossibleRegion() );
}

sitk::Image RayMasking::isolateIntracranialVoxels(sitk::Image &initialMask, sitk::Image &boneMask)
{
    /* ------ Init Variables ------ */

    // Cast sitk images to itk image
    MaskImageType::Pointer itkInitialMask = MaskImageType::New();
    sitkToBinaryItk( initialMask, itkInitialMask );

    MaskImageType::Pointer itkBoneMask = MaskImageType::New();
    sitkToBinaryItk( boneMask, itkBoneMask );

    // Create output mask
    MaskImageType::Pointer outputMask = MaskImageType::New();
    MaskImageType::RegionType region;
    region.SetSize( itkInitialMask->GetLargestPossibleRegion().GetSize() );
    region.SetIndex( itkInitialMask->GetLargestPossibleRegion().GetIndex() );
    outputMask->SetRegions( region );
    outputMask->Allocate();
    outputMask->FillBuffer(0);

    // Init variables required for extract filter
    MaskImageType::IndexType start = { 0,0,0 };
    MaskImageType::RegionType inputRegion = itkInitialMask->GetLargestPossibleRegion();
    MaskImageType::SizeType size = inputRegion.GetSize();
    size[2] = 0; // extract along z direction

    // Init extract filter for initial mask
    // Extract filter is used to extract a 2D slice from a 3D image
    // TODO: Wrap this in a function
    ExtractFilterType::Pointer extractFilter = ExtractFilterType::New();
    extractFilter->SetDirectionCollapseToSubmatrix();
    extractFilter->SetInput( itkInitialMask );
    MaskImageType::RegionType desiredRegion;
    desiredRegion.SetSize(  size  );

    // Init extract filter for bone mask
    ExtractFilterType::Pointer extractFilterBone = ExtractFilterType::New();
    extractFilterBone->SetDirectionCollapseToSubmatrix();
    extractFilterBone->SetInput( itkBoneMask );
    MaskImageType::RegionType desiredRegionBone;
    desiredRegionBone.SetSize(  size  );

    // Init extract filter for output
    ExtractFilterType::Pointer extractFilterOutput = ExtractFilterType::New();
    extractFilterOutput->SetDirectionCollapseToSubmatrix();
    extractFilterOutput->SetInput( outputMask );
    MaskImageType::RegionType desiredRegionOutput;
    desiredRegionOutput.SetSize(  size  );
    
    int nSlices = inputRegion.GetSize()[2];
    constexpr unsigned int intersectionTreshold = 6;

    // Layout is used with tile mask filter which is used to stack 2d slices into a 3d image
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

        /*------ Extract 2d slice from 3d image ------*/

        // Specify slice idx to be extracted from 3d image
        start[2] = sliceIdx;
        
        // Extract 2d slice from initial mask
        desiredRegion.SetIndex( start );
        extractFilter->SetExtractionRegion( desiredRegion );
        extractFilter->Update();
        initialMaskSlice = extractFilter->GetOutput();

        // Extract 2d slice from bone mask
        desiredRegionBone.SetIndex( start );
        extractFilterBone->SetExtractionRegion( desiredRegionBone );
        extractFilterBone->Update();
        boneMaskSlice = extractFilterBone->GetOutput();

        // Extract 2d slice from output mask
        desiredRegionOutput.SetIndex( start );
        extractFilterOutput->SetExtractionRegion( desiredRegionOutput );
        extractFilterOutput->Update();
        outputMaskSlice = extractFilterOutput->GetOutput();

        /*------ Init iterators ------*/

        // Ray casting
        // Init iterators
        constIterType it(initialMaskSlice, initialMaskSlice->GetLargestPossibleRegion());
        iterType itOutput(outputMaskSlice, outputMaskSlice->GetLargestPossibleRegion());

        // Init chain code which will be used to encode ray directions
        chainCodeType::Pointer chainCode = chainCodeType::New();
        // Path iter will be used to iterate on image starting from a certain position and following the
        // path specified in chainCode
        pathConstIterType pathIter(boneMaskSlice, chainCode);

        // Index at which the image iterator is currently
        MaskImage2DType::IndexType currentIndex;
        // Number of intersection that rays casted from a pixel made with bone pixels (skull)
        unsigned int numberOfIntersections;
        // Max number of steps that one can take along ray directions without going out of bounds
        std::vector<long> numberOfSteps;

        // Ray directions in freeman codes
        std::vector<unsigned int> pathDirections = { 1,2,3,4,5,6,7,8 };

        /* ----- Iterate over image ----- */

        it.GoToBegin();
        itOutput.GoToBegin();

        while ( !it.IsAtEnd() )
        {
            // If current pixel on mask is 0 it is also 0 in outputmask, so continue
            if (it.Get() == 0) { ++it; ++itOutput; continue; }
            
            numberOfIntersections = 0;
            currentIndex = it.GetIndex();
            // Init numberOfSteps for current pixel
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
                // Fill the chain code vector with same chain code
                chainCode->FillWithSteps(numberOfSteps[i-1], i); // Custom

                // Iterate by following the path on the image
                pathIter.GoToBegin();
                while ( !pathIter.IsAtEnd() )
                {
                    // If coincides with bone pixel
                    if ( pathIter.Get() == 1)
                    {
                        ++numberOfIntersections;
                        break;
                    }
                    ++pathIter;
                }
                chainCode->Clear();
            }

            // If, for the current pixel, at least 7 rays intersected with bone pixels out of 8, then that pixel is interpreted as
            // inside skull and is marked as 1 in output mask
            if (numberOfIntersections > intersectionTreshold) { itOutput.Set(1); }
            ++it; ++itOutput;
        }
        // To use the output mask slice independent from the filter that owns it, the mask slice should be disconnected from filter's pipeline
        outputMaskSlice->DisconnectPipeline();
        // Put 2d slice into the final 3d image
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
        //sitk::Show( sitk::Image( outputSlice ) );
        outputSlice->DisconnectPipeline();
        tileFilter->SetInput(i, outputSlice);
    }

    InputImageType::Pointer itkOutput = tileFilter->GetOutput();
    itkOutput->DisconnectPipeline();
    sitk::Image output = sitk::Image( itkOutput );
    sitk::Show(output);
    return output;
}