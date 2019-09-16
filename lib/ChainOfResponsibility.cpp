#include "../includes/ChainOfResponsibility.h"
#include "../includes/Masking/Masking.h"
#include <iostream>

void Chain::addCommandByName(const std::string& name)
{
    if (name == "masking")
    {
        std::cout << "Adding masking command." << std::endl;

        std::shared_ptr<Command> maskingCommand = std::make_shared<Masking>(std::shared_ptr<Chain>(this));
        commandList.push_back(maskingCommand);
    } 
    else
    {
	    std::cout << "Wrong command type" << std::endl;
    }
}

bool Chain::execute()
{
    if (commandList.size() == 0)
    {
        std::cout << "No command added yet. Exiting." << std::endl;
        return false;
    }

    bool ok = true;

    std::cout << "Starting execution." << std::endl;
    for (std::shared_ptr<Command>& command : commandList)
    {
        ok = command->execute();
        if (ok == false)
        {
            break;
        }
    }
    return ok;
}

void Chain::setParameters(Parameters &params) { this->params = params; }
Parameters Chain::getParameters() const { return params; }

void Chain::setMask(sitk::Image &mask) { this->mask = mask; }
sitk::Image Chain::getMask() const { return mask; }

void Chain::setImage(sitk::Image &image) { this->image = image; }
sitk::Image Chain::getImage() const { return image; }

Parameters::Parameters()
{
    params["maskingStrategy"] = "rayCasting";
    params["regularMaskingRegionGrowingSeedType"] = "center";
    params["regularMaskingRegionGrowingLowerThreshold"] = "0";
    params["regularMaskingRegionGrowingUpperThreshold"] = "80";
}
void Parameters::setParameter(std::string key, std::string value) { params[key] = value; }
std::string Parameters::getValue(std::string key) { return params[key]; }
void Parameters::validateParameters() {}

