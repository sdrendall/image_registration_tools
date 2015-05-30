#include "image_slicing.h"
#include "image_io.h"
#include "optionparser.h"
#include <iostream>

#include "itkImage.h"

#ifndef SLICE_ATLAS
#define SLICE_ATLAS

template<typename IMAGE_TYPE>
typename IMAGE_TYPE::Pointer get_atlas_slice(const char* atlas_path, const int slice_index, const int axis_to_collapse=0) {;
    // Returns an image of IMAGE_TYPE containing a slice from the atlas at atlas_path
    // Creates coronal slices by default
    typedef itk::Image<typename IMAGE_TYPE::PixelType, 3> AtlasImageType;
    
    // Load the atlas
    typename AtlasImageType::Pointer atlas = load_image<AtlasImageType>(atlas_path);

    // Return a slice of the atlas
    return extract_image_slice<IMAGE_TYPE>(atlas, slice_index, axis_to_collapse);
}

#endif
