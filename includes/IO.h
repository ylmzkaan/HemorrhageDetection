#pragma once
#include <SimpleITK.h>
#include <iostream>
#include <stdlib.h>

namespace sitk = itk::simple;

sitk::Image readDicomSeries(const std::string& dicomDirectory);
