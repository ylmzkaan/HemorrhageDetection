#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "sitkImage.h"

namespace sitk = itk::simple;

class Command;


/*
Parameters class provides an interface for the user to configure the commands within the chain by specifying key-value pairs.
*/
class Parameters
{
    private:
	std::unordered_map<std::string, std::string> params;

    public:
	Parameters();
	void setParameter(std::string key, std::string value);
	std::string getValue(std::string key);

	private:
	void validateParameters();
};

/*
Chain class provides a way to execute commands in the order that they are added to the Chain. Commands added to the Chain class can
manipulate the image and mask attributes. For example, a "masking" command might use image to create a mask which will be used by another command
called "classification" to classify only the pixels that belong to the mask.

Currently, only "masking" command is implemented and it can be added to the chain using the function addCommandByName("masking")

One has to set the image before calling the execute() method of the Chain class. When execute() function is called, it will iterate over the
commandList and call each object's execute method. These objects should be a subclass of Command.
*/
class Chain
{
    private:
	sitk::Image image;
	sitk::Image mask;
	std::vector<std::shared_ptr<Command>> commandList = std::vector<std::shared_ptr<Command>>{};
	Parameters params;

    public:	
	void addCommandByName(const std::string &name);
	bool execute();
	
	void setParameters(Parameters &params);
	Parameters getParameters() const;
	
	void setMask(sitk::Image &mask);
	sitk::Image getMask() const;
	
	void setImage(sitk::Image &image);
	sitk::Image getImage() const;
};

/*
Any type of operation which will be executed by the Chain class should be a subclass of Command class and implemented the execute() function.
*/
class Command
{
    protected:
	std::shared_ptr<Chain> chain;
	
    public:
	Command(std::shared_ptr<Chain> chain) : chain(chain) {}
	virtual bool execute() = 0;
};

