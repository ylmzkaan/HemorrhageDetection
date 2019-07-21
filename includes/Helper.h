#pragma once
#include "SimpleITK.h"
#include "itkImage.h"

namespace sitk = itk::simple;
using namespace std;

sitk::CastImageFilter               caster;

bool sitkImageToItkImage(sitk::Image &image, itk::Image< int, 3 >::Pointer itkImage);