/*
Author: Kaan Yilmaz
E-mail: kaan-yilmaz@outlook.com.tr

This program is an implementation of the brain ct segmentation method proposed in https://arxiv.org/pdf/1906.09726.pdf (Still under development).

The objective is to segment the brain's computed tomographies where the segmented area is the intracranial area located inside the skull. Segmentation of 
this area is important as most of the patologies occur in this area.

The segmentation/masking algorithm implemented here relies on ray tracing. Because region of interest(intracranial area) has values between
-10 and 100 and it is mostly enclosed by the skull(will be called bone in variable naming) whose pixel values are between 300 and 1500. 

However, there are other pixels whose values overlap with the pixel intensity range of intracranial pixels but don't belong the intracranial area.
Thus, embedding the knowledge that intracranial area has intensities between -10 - 100 AND is enclosed by skull significantly improves the segmentation
performance.

The main idea of the algorithm is that if 7 of 8 rays (equally spread with 45 degrees increments) casted from a pixel collides with a skull pixel, 
then one can conclude that the pixel is enclosed by skull and belongs to intracrnial area.


The program flow implements the chain of responsibility design pattern. A chain has commands which can be "masking", "classification" etc. 
But for the moment only the "masking" command is implemented.

One can configure the chain of responsibility parameters using the Parameters class. It is possible to change the type of segmentation method and
configure its hyperparameters.

When its execute() method called, the instance of chain will start to execute the command in the order they are added to the chain.
*/
#include "../includes/IO.h"
#include "../includes/ChainOfResponsibility.h"
#include <iostream>
#include <sitkImageOperators.h>

#define N_DEBUG

int main(int argc, char* argv[])
{
    if ( argc < 2 ) 
    {
        std::cerr << "Only one dicom directory should be specified which is to be read." << std::endl;
        return 1;
    }

    // Read the directory containing the dicom files
    sitk::Image dicomSlices = readDicomSeries(argv[1]);
    
    // Get default parameters
    Parameters params = Parameters();

    // Init chain and add commands
    Chain hemorrhageAnalysis = Chain();
    hemorrhageAnalysis.setParameters(params);
    hemorrhageAnalysis.setImage(dicomSlices);
    hemorrhageAnalysis.addCommandByName("masking");

    hemorrhageAnalysis.execute(); 

    //#ifndef NDEBUG
    sitk::Image mask = hemorrhageAnalysis.getMask();
    sitk::Show(mask, "Initial mask");
    //#endif
}