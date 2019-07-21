#include "../includes/IO.h"
#include <iostream>

using namespace std; 

sitk::Image readDicomSeries(const std::string& dicomDirectory) 
{
    cout << "Reading DICOM series." << endl;

    sitk::ImageSeriesReader reader;
    reader.MetaDataDictionaryArrayUpdateOn();
    reader.LoadPrivateTagsOn();
    const std::vector<std::string> dicom_names = sitk::ImageSeriesReader::GetGDCMSeriesFileNames( dicomDirectory );
    reader.SetFileNames( dicom_names );

    sitk::Image image = reader.Execute();

    cout << "DICOM series are read successfully." << endl;

    return image;
}
