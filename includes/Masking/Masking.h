#pragma once
#include "../ChainOfResponsibility.h"
#include "MaskingMethods/RegularMasking.h"
#include "MaskingMethods/RayMasking.h"
#include "MaskingStrategy.h"
#include <memory>
#include <iostream>
#include <stdexcept>
#include <sitkImage.h>
#include <SimpleITK.h>
#include <string>

namespace sitk = itk::simple;

/*
Any operation that will do masking operations whould inherit this class
*/
class Masking : public Command
{
    private:
	std::shared_ptr<MaskingStrategy> maskingMethod;

    public:
	Masking(std::shared_ptr<Chain> chain);
	bool execute();
};