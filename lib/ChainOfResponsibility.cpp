#include "../includes/ChainOfResponsibility.h"
#include "../includes/Masking/Masking.h"
#include <iostream>

void Chain::addCommandByName(const string& name)
{
    if (name == "masking")
    {
        cout << "Adding masking command." << endl;

        shared_ptr<Command> maskingCommand = make_shared<Masking>(shared_ptr<Chain>(this));
        commandList.push_back(maskingCommand);
    } 
    else
    {
	    cout << "Wrong command type" << endl;
    }
}

bool Chain::execute()
{
    if (commandList.size() == 0)
    {
        cout << "No command added yet. Exiting." << endl;
        return false;
    }

    bool ok = true;

    cout << "Starting execution." << endl;
    for (shared_ptr<Command>& command : commandList)
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
void Parameters::setParameter(string key, string value) { params[key] = value; }
string Parameters::getValue(string key) { return params[key]; }
void Parameters::validateParameters() {}

