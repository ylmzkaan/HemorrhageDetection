#include "../includes/IO.h"
#include "../includes/Masking/Masking.h"
#include "../includes/ChainOfResponsibility.h"
#include <iostream>
#include <sitkImageOperators.h>

using namespace std;

int main(int argc, char* argv[])
{
    if ( argc < 2 ) 
    {
        cerr << "Only one dicom directory should be specified which is to be read.";
        return 1;
    }

    sitk::Image dicomSlices = readDicomSeries(argv[1]);
    
    Parameters params = Parameters();

    Chain hemorrhageAnalysis = Chain();
    hemorrhageAnalysis.setParameters(params);
    hemorrhageAnalysis.setImage(dicomSlices);
    hemorrhageAnalysis.addCommandByName("masking");

    hemorrhageAnalysis.execute(); 

    #ifndef NDEBUG
    sitk::Image mask = hemorrhageAnalysis.getMask();
    sitk::Show(mask, "Initial mask");
    #endif
}
