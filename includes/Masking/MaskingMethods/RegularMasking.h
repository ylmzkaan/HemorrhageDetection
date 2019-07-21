#pragma once
#include "../MaskingStrategy.h"
#include <iostream>
#include <stdexcept>
#include <SimpleITK.h>

using namespace std;
namespace sitk = itk::simple;

class RegularMasking : public MaskingStrategy 
{
    private:
	const string regionGrowingSeedType;
	const int regionGrowingLowerThreshold;
	const int regionGrowingUpperThreshold;

	unsigned int nSlices;

	sitk::ConnectedThresholdImageFilter     regionGrower;
    sitk::MaskImageFilter                   maskFilter;
    sitk::ThresholdImageFilter              thresholdFilter;
    sitk::MedianImageFilter                 medianFilter;
    sitk::ExtractImageFilter                extractImage;

    public:
	RegularMasking(sitk::Image &image, const string &regionGrowingSeedType,
					const int &regionGrowingLowerThreshold,
					const int &regionGrowingUpperThreshold);
	void run()
	{
		sitk::Image brainParenchyma = getBrainParenchyma();
	}

	private:
	sitk::Image getBrainParenchyma();
};