#include "../includes/Masking/Masking.h"

namespace sitk = itk::simple;

bool Masking::execute()
{
    maskingMethod->run(); 
    this->chain->setMask(maskingMethod->getMask());
    return true;
}

Masking::Masking(std::shared_ptr<Chain> chain) : Command(chain)
{   
    sitk::Image image = this->chain->getImage();
    Parameters params = this->chain->getParameters();

    if (params.getValue("maskingStrategy") == "regular")
    {
        auto regionGrowingSeed = params.getValue("regularMaskingRegionGrowingSeedType");
        if (regionGrowingSeed != "center")
        {
            std::ostringstream stream;
            stream << regionGrowingSeed << " specified as regularMaskingRegionGrowingSeed parameter is a wrong input argument."; 
            throw std::invalid_argument(stream.str());
        }

        double regionGrowingLowerThreshold = stod(params.getValue("regularMaskingRegionGrowingLowerThreshold"), nullptr);
        double regionGrowingUpperThreshold = stod(params.getValue("regularMaskingRegionGrowingUpperThreshold"), nullptr);
        maskingMethod = std::make_shared<RegularMasking>(image, regionGrowingSeed, regionGrowingLowerThreshold, regionGrowingUpperThreshold);
    }
    else if ( params.getValue("maskingStrategy") == "rayCasting" )
    {
        maskingMethod = std::make_shared<RayMasking>(image);
    }
    else
    {
        std::cout << "Wrong masking strategy specified in parameters." << std::endl;
    }
}