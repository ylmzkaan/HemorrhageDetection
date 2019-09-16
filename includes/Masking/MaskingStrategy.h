#pragma once
#include <SimpleITK.h>
 
namespace sitk = itk::simple;

class MaskingStrategy
{
    protected:
	sitk::Image image;
	sitk::Image mask;

    public:
	MaskingStrategy(sitk::Image &image) : image(image) {};
	sitk::Image& getMask() { return mask; };
	virtual void run() = 0;
};