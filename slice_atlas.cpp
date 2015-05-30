#include "slice_atlas.h"

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
    ATLAS_PATH,
    OUTPUT_PATH,
    SLICE_INDEX,
    SLICE_AXIS
};


const option::Descriptor usage[] = {
    {UNKNOWN, 0, "", "", Arg::Unknown, "USAGE: image_to_image_registration [options]\n\n"
                                       "Options: "},
    {HELP, 0, "h", "help", Arg::None, "--help, -h \tDisplay this help message and exit"},
    {ATLAS_PATH, 0, "a", "atlas", Arg::Required, "--atlas, -a path \tPath to the atlas to slice from"},
    {OUTPUT_PATH, 0, "o", "output", Arg::Required, "--output, -o path \tPath to save the output moving image"},
    {SLICE_INDEX, 0, "i", "index", Arg::Required, "--index, -i index \tIndex of the desired slice on the slice axis"},
    {SLICE_AXIS, 0, "a", "axis", Arg::Required, "--axis, -a int \tIndex of the axis to slice along"},
    {0,0,0,0,0,0}
};

int main(int argc, char** argv ) {
    // Parse input
    argv += (argc>0);
    argc -= (argc>0);

    option::Stats stats(usage, argc, argv);
    option::Option* options = new option::Option[stats.options_max];
    option::Option* buffer = new option::Option[stats.buffer_max];
    option::Parser parse(usage, argc, argv, options, buffer);

    if (options[HELP]) {
        option::printUsage(cout, usage);
        return 1;
    }

    // Check for required arguments
    if (!options[ATLAS_PATH] || !options[OUTPUT_PATH] || !options[SLICE_INDEX]) {
        cout << "Insufficient arguments!!" << endl;
        cout << "An atlas path, output path, and slice index must be specified" << endl << endl;
        option::printUsage(cout, usage);
        return 1;
    }

    // Extract slice
    const int slice_index = atoi(options[SLICE_INDEX].arg);
    const char* atlas_path = options[ATLAS_PATH].arg; 
    const char* output_path = options[OUTPUT_PATH].arg;
    const int axis_to_collapse = options[SLICE_INDEX]? atoi(options[SLICE_INDEX].arg) : 0; // Default to coronal slices


    typedef itk::Image<float, 2> SliceImageType;
    SliceImageType::Pointer atlas_slice = get_atlas_slice<SliceImageType>(atlas_path, slice_index, axis_to_collapse);

    write_image<SliceImageType>(atlas_slice, output_path);
    return 0;
}
