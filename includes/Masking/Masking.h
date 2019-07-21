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

using namespace std;
namespace sitk = itk::simple;

class Masking : public Command
{
    private:
	shared_ptr<MaskingStrategy> maskingMethod;

    public:
	Masking(shared_ptr<Chain> chain);
	bool execute();
};