#include "itkImage.h"
#include "itkExtractImageFilter.h"

#include "image_io.h"

#ifndef IMAGE_SLICING
#define IMAGE_SLICING


template<typename IMAGE_TYPE>
typename IMAGE_TYPE::Pointer extract_image_slice(
    const typename itk::Image<typename IMAGE_TYPE::PixelType, IMAGE_TYPE::ImageDimension + 1>::Pointer image_to_slice,
    const int slice_index,
    const int axis_to_collapse
) {
    typedef itk::Image<typename IMAGE_TYPE::PixelType, IMAGE_TYPE::ImageDimension + 1> SlicedImageType;
    typedef itk::ExtractImageFilter<SlicedImageType, IMAGE_TYPE> SliceImageFilterType;

    // Determine Slice Size
    typename SlicedImageType::RegionType entire_atlas_region = image_to_slice->GetLargestPossibleRegion();
    typename SlicedImageType::SizeType sliced_image_size = entire_atlas_region.GetSize();
    typename SlicedImageType::SizeType slice_size = sliced_image_size;
    slice_size[axis_to_collapse] = 0;  // 0 tells the ExtractionFilter to return an Image without that dimension

    // Get Start Point
    typename SlicedImageType::IndexType slice_start_index = entire_atlas_region.GetIndex();
    slice_start_index.Fill(0);
    slice_start_index[axis_to_collapse] = slice_index;

    // Initialize a slice region
    typename SlicedImageType::RegionType slice_region(slice_start_index, slice_size);

    // Extract the slice
    typename SliceImageFilterType::Pointer extraction_filter = SliceImageFilterType::New();
    extraction_filter->SetInput(image_to_slice);
    extraction_filter->SetDirectionCollapseToIdentity();

    extraction_filter->SetExtractionRegion(slice_region);
    extraction_filter->Update();
    return extraction_filter->GetOutput();
}


#endif
