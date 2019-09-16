#include "../includes/IO.h"
#include <iostream>

sitk::Image readDicomSeries(const std::string& dicomDirectory) 
{
    std::cout << "Reading DICOM series." << std::endl;

    sitk::ImageSeriesReader reader;
    reader.MetaDataDictionaryArrayUpdateOn();
    reader.LoadPrivateTagsOn();
    const std::vector<std::string> dicom_names = sitk::ImageSeriesReader::GetGDCMSeriesFileNames( dicomDirectory );
    reader.SetFileNames( dicom_names );

    sitk::Image image = reader.Execute();

    std::cout << "DICOM series are read successfully." << std::endl;

    return image;
}
