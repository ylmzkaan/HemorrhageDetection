#include "../includes/Masking/MaskingMethods/RegularMasking.h"

namespace sitk = itk::simple;

/*
This masking algorithm based on region growing with lower and upper threshold specified in parameters with 
regionGrowingLowerThreshold and regionGrowingUpperThreshold. If regionGrowingSeedType is "center" the seed is at x=230 y=340 z=number_of_slices/2.
*/
RegularMasking::RegularMasking(sitk::Image &image, const std::string &regionGrowingSeedType,
                                const int &regionGrowingLowerThreshold,
                                const int &regionGrowingUpperThreshold) 
: MaskingStrategy(image), regionGrowingSeedType(regionGrowingSeedType),
    regionGrowingLowerThreshold(regionGrowingLowerThreshold),
    regionGrowingUpperThreshold(regionGrowingUpperThreshold) {}


sitk::Image RegularMasking::getBrainParenchyma()
{
    std::cout << "Applying region growing to extract brain parenchyma." << std::endl;

    image = medianFilter.Execute(image);

    // Setup Region Growing filter
    unsigned int idxCenterSlice = (unsigned int) image.GetDepth() / 2;
    if (regionGrowingSeedType == "center") { regionGrower.SetSeed(std::vector<unsigned int> {230,340,idxCenterSlice}); }
    regionGrower.SetLower(regionGrowingLowerThreshold);
    regionGrower.SetUpper(regionGrowingUpperThreshold);
    regionGrower.SetReplaceValue(1);
    regionGrower.SetConnectivity(sitk::ConnectedThresholdImageFilter::ConnectivityType::FaceConnectivity);
    
    sitk::Image brainParenchymaMask = regionGrower.Execute(image);
    sitk::Image brainParenchyma = maskFilter.Execute(image, brainParenchymaMask, 0, 1);
    //brainParenchyma = thresholdFilter.Execute(image, 60, 80, 0);
    
    #ifndef NDEBUG
    sitk::Show(brainParenchyma, "Brain Parenchyma");
    #endif

    std::cout << "Brain parenchyma is extracted." << std::endl;
    return brainParenchyma;
}