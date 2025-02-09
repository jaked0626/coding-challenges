#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unistd.h>

#include "compress.h"
#include "decompress.h"

const char* PROGRAM_NAME;

void print_usage(std::ostream& stream)
{
    stream << "jzip compresses files or expands them depending on the file type passed.\n"
           << "If the file type is a text file or comparable file, it will generate a <filename>.jzip file with compressed contents.\n"
           << "If the file type is a file ending in .jzip, it will decompress the file.\n\n"
           << "Usage: " << PROGRAM_NAME << " <-oh> " <<"<filepath>\n"
           << "\t-h display this usage information.\n";
}

bool process_arguments(std::ifstream& infile, std::ofstream& outfile, bool& compress, int argc, char* argv[])
{
    PROGRAM_NAME = argv[0];
    int opt {};
    const char* opt_flags { "h" };
    
    while ((opt = getopt(argc, argv, opt_flags)) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(std::cout);
            std::exit(0);
        case '?':
            print_usage(std::cerr);
            return false;
        default:
            return false;
        }
    }

    // when a infilepath is given we want to error check and unload to istream
    if (optind < argc)
    {
        std::string infilepath { argv[optind] };
        std::filesystem::path infile_system_path { infilepath };

        // check if file is valid. 
        if (!std::filesystem::exists(infile_system_path))
        {
            std::cerr << "Error: file " << infilepath << " does not exist.\n";
            return false;
        }
        if (!std::filesystem::is_regular_file(infile_system_path))
        {
            std::cerr << "Error: file " << infilepath << " is not a regular file.\n";
            return false;
        }

        infile.open(infilepath, std::ios::in | std::ios::binary);

        if (!infile.is_open() || infile.bad())
        {
            std::cerr << "Error: file " << infilepath << " could not be opened.\n";
            return false;
        }

        // based on the file type of the input file, we decide the name of the output 
        // file and whether the operation we perform on it will be compression or decompression. 
        std::string outfilepath {};
        if (infile_system_path.extension().string() == ".jzip")
        {
            outfilepath = infilepath.substr(0, infilepath.size() - 5);
            compress = false;
        }
        else 
        {
            outfilepath = infilepath + ".jzip";
            compress = true;
        }

        // ensure output file does not already exist.
        std::filesystem::path outfile_system_path { outfilepath };
        if (std::filesystem::exists(outfile_system_path))
        {
            std::cerr << "Error: the file " << outfilepath << " already exists. Delete or rename this file before proceeding\n";
            return false;
        }

        outfile.open(outfilepath);
    }
    else 
    {
        std::cerr << "Error: no file path passed.\n";
        print_usage(std::cerr);
    }

    return true;
}

int main(int argc, char* argv[])
{
    // program inputs 
    std::ifstream infile {};
    std::ofstream outfile {};
    bool compress {};
    
    // process args
    bool ok {};
    ok = process_arguments(infile, outfile, compress, argc, argv);
    if (!ok)
    {
        return 1;
    }

    if (compress)
    {
        ok = compress_file(infile, outfile);
    }
    else
    {
        ok = decompress_file(infile, outfile);
    }

    return ok ? 0 : 1;
}