#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkTransformFileWriter.h"

#ifndef IMAGE_IO
#define IMAGE_IO

template<typename IMAGE_TYPE>
typename IMAGE_TYPE::Pointer load_image(const char* image_path) {
    // Reads an image from image_path into an itk image of type IMAGE_TYPE
    typedef itk::ImageFileReader<IMAGE_TYPE> ImageReaderType;
    typename ImageReaderType::Pointer image_reader = ImageReaderType::New();

    image_reader->SetFileName(image_path);
    image_reader->Update();
    return image_reader->GetOutput();
}

template<typename IMAGE_TYPE>
void write_image(const typename IMAGE_TYPE::Pointer, const char* image_path) {
    // Writes an itk image of type IMAGE_TYPE to the location specified in image_path
    // The image file format is determined using the suffix of image_path
    typedef itk::ImageFileWriter<IMAGE_TYPE> ImageWriterType;
    typename ImageWriterType::Pointer image_writer = ImageWriterType::New();
    image_writer->SetFileName(image_path);
    image_writer->Update();
}

template<typename TRANSFORM_PTR_TYPE>
void write_transform(const TRANSFORM_PTR_TYPE transform, const char* image_path) {
    typedef itk::TransformFileWriterTemplate<double> TransformWriterType;
    TransformWriterType::Pointer transform_writer = TransformWriterType::New();

    transform_writer->AddTransform(transform);
    transform_writer->SetFileName(image_path);
    transform_writer->Update();
}
#endif
