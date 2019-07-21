#include "../includes/Helper.h"

bool sitkImageToItkImage(sitk::Image &image, itk::Image< int, 3 >::Pointer itkImage)
{
    caster.SetOutputPixelType( sitk::sitkInt32 );
    sitk::Image imageInt = caster.Execute( image );

    itkImage->SetBufferedRegion( itkImage->GetLargestPossibleRegion() );
    itkImage = dynamic_cast <itk::Image< int, 3 >*>( imageInt.GetITKBase() );
    itkImage->DisconnectPipeline();

    if ( itkImage.IsNull() )
    {
        cerr << "Unexpected error converting SimpleITK image to ITK image!" << endl;
        return false;
    }

    return true;
}
