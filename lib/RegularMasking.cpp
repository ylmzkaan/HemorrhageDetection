#include "../includes/Masking/MaskingMethods/RegularMasking.h"

using namespace std;
namespace sitk = itk::simple;

RegularMasking::RegularMasking(sitk::Image &image, const string &regionGrowingSeedType,
                                const int &regionGrowingLowerThreshold,
                                const int &regionGrowingUpperThreshold) 
: MaskingStrategy(image), regionGrowingSeedType(regionGrowingSeedType),
    regionGrowingLowerThreshold(regionGrowingLowerThreshold),
    regionGrowingUpperThreshold(regionGrowingUpperThreshold) {}

sitk::Image RegularMasking::getBrainParenchyma()
{
    cout << "Applying region growing to extract brain parenchyma." << endl;

    image = medianFilter.Execute(image);

    // Setup Region Growing filter
    unsigned int idxCenterSlice = (unsigned int) image.GetDepth() / 2;
    if (regionGrowingSeedType == "center") { regionGrower.SetSeed(vector<unsigned int> {230,340,idxCenterSlice}); }
    regionGrower.SetLower(regionGrowingLowerThreshold);
    regionGrower.SetUpper(regionGrowingUpperThreshold);
    regionGrower.SetReplaceValue(1);
    regionGrower.SetConnectivity(sitk::ConnectedThresholdImageFilter::ConnectivityType::FaceConnectivity);
    
    sitk::Image brainParenchymaMask = regionGrower.Execute(image);
    sitk::Image brainParenchyma = maskFilter.Execute(image, brainParenchymaMask, 0, 1);
    brainParenchyma = thresholdFilter.Execute(image, 60, 80, 0);
    
    #ifndef NDEBUG
    sitk::Show(brainParenchyma, "Brain Parenchyma");
    #endif

    cout << "Brain parenchyma is extracted." << endl;
    return brainParenchyma;
}
