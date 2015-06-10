#include "apply_transform.h"

using namespace std;

// option parsing
struct Arg: public option::Arg
 {
   static void printError(const char* msg1, const option::Option& opt, const char* msg2)
   {
     fprintf(stderr, "ERROR: %s", msg1);
     fwrite(opt.name, opt.namelen, 1, stderr);
     fprintf(stderr, "%s", msg2);
   }

   static option::ArgStatus Unknown(const option::Option& option, bool msg)
   {
     if (msg) printError("Unknown option '", option, "'\n");
     return option::ARG_ILLEGAL;
   }

   static option::ArgStatus Required(const option::Option& option, bool msg)
   {
     if (option.arg != 0)
       return option::ARG_OK;

     if (msg) printError("Option '", option, "' requires an argument\n");
     return option::ARG_ILLEGAL;
   }

   static option::ArgStatus Numeric(const option::Option& option, bool msg)
   {
     char* endptr = 0;
     if (option.arg != 0 && strtol(option.arg, &endptr, 10)){};
     if (endptr != option.arg && *endptr == 0)
       return option::ARG_OK;

     if (msg) printError("Option '", option, "' requires a numeric argument\n");
     return option::ARG_ILLEGAL;
   }
 };


enum optionIndex {
    UNKNOWN,
    HELP,
    MOVING_IMAGE,
    OUTPUT_PATH,
    TRANSFORM_PATH,
    INVERT_TRANSFORM,
    INPUT_TRANSFORM_TYPE
};


const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", Arg::Unknown, "USAGE: image_to_image_registration [options]\n\n"
                                       "Options: "},
    {HELP, 0, "h", "help", Arg::None, "--help, -h \tDisplay this help message and exit"},
    {MOVING_IMAGE, 0, "m", "moving", Arg::Required, "--moving, -m path \tPath to the moving image to apply the transform to."},
    {OUTPUT_PATH, 0, "o", "output", Arg::Required, "--output, -o path \tPath to save the transformed moving image"},
    {TRANSFORM_PATH, 0, "t", "transform", Arg::Required, "--transform, -t path \tPath to the transform to apply"},
    {INVERT_TRANSFORM, 0, "i", "invert", Arg::None, "--invert, -i \tInvert the given transform"},
    {INPUT_TRANSFORM_TYPE, 0, "r", "transform_type", Arg::Required, "--transform_type -r type \tType of the transform to be applied\n"
                                                                    "Default: itk::CompositeTransform"},
    {0,0,0,0,0,0}
};


int main(int argc, char** argv) {
    // parse options
    argv += (argc > 0);
    argc -= (argc > 0);

    option::Stats stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    option::Option* buffer = new option::Option[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (options[HELP]) {
        option::printUsage(cout, usage);
        return 1;
    }

    if (!options[MOVING_IMAGE] || !options[OUTPUT_PATH] || !options[TRANSFORM_PATH]) {
        cout << "Insufficient Arguments!!" << endl;
        cout << "Please specify a moving image, an output path, and a transform to apply." << endl;
        option::printUsage(cout, usage);
        return 1;
    }

    // Read image
    typedef itk::Image<float, 2> ImageType; 
    ImageType::Pointer image = load_image<ImageType>(options[MOVING_IMAGE].arg);

    // Read transform
    COMPOSITE_TRANSFORM_TYPE::Pointer transform = read_transform<COMPOSITE_TRANSFORM_TYPE>(options[TRANSFORM_PATH].arg);

    // If not inverse, apply normal transform
    if (!options[INVERT_TRANSFORM]) {
        image = apply_transform<ImageType, COMPOSITE_TRANSFORM_TYPE>(image, transform);
    } else {
    // TODO: Temp message
    cout << "Inverted transforms not supported yet!" << endl;
    }

    // Else, invert transform
        // If has method (GetInverse) use inverse

        // Else, use deformation field



    return 0;
}
