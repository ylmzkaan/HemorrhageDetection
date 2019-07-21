#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "sitkImage.h"

using namespace std;
namespace sitk = itk::simple;

class Command;

class Parameters
{
    private:
	unordered_map<string, string> params;

    public:
	Parameters();
	void setParameter(string key, string value);
	string getValue(string key);

	private:
	void validateParameters();
};

class Chain
{
    private:
	sitk::Image image;
	sitk::Image mask;
	vector<shared_ptr<Command>> commandList = vector<shared_ptr<Command>>{};
	Parameters params;

    public:	
	void addCommandByName(const string &name);
	bool execute();
	
	void setParameters(Parameters &params);
	Parameters getParameters() const;
	
	void setMask(sitk::Image &mask);
	sitk::Image getMask() const;
	
	void setImage(sitk::Image &image);
	sitk::Image getImage() const;
};

class Command
{
    protected:
	shared_ptr<Chain> chain;
	
    public:
	Command(shared_ptr<Chain> chain) : chain(chain) {}
	virtual bool execute() = 0;
};

