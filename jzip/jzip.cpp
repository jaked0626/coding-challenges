#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <unordered_map>

const char* program_name;

void print_usage(std::ostream& stream)
{
    stream << "jzip compresses files or expands them depending on the filetype passed.\n"
           << "Usage: " << program_name << " <filepath>\n"
           << "\t-h display this usage information.\n";
}

bool process_arguments(std::fstream& file, int argc, char* argv[])
{
    program_name = argv[0];
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

    // when a filepath is given we want to error check and unload to istream
    if (optind < argc)
    {
        std::string filepath { argv[optind] };
        std::filesystem::path file_system_path { filepath };

        // check if file is valid. 
        if (!std::filesystem::exists(file_system_path))
        {
            std::cerr << "Error: file " << filepath << " does not exist.\n";
            return false;
        }
        if (!std::filesystem::is_regular_file(file_system_path))
        {
            std::cerr << "Error: file " << filepath << " is not a regular file.\n";
            return false;
        }

        file.open(filepath, std::ios::in | std::ios::binary);

        if (!file.is_open())
        {
            std::cerr << "Error: file " << filepath << " could not be opened.\n";
            return false;
        }
    }
    else 
    {
        std::cerr << "Error: no file path passed.\n";
        print_usage(std::cerr);
    }

    return true;
}

bool count_chars(std::fstream& file, std::unordered_map<char, int>& char_counts) 
{
    char c {};
    while (file.get(c))
    {
        ++char_counts[c];
    }
    if (file.eof())
    {
        file.clear();
        file.seekg(0, std::ios::beg);
    }
    else
    {
        return false;
    }

    return true;
}

bool create_huffman_tree(const std::unordered_map<char, int>& char_counts)
{
    return true;
}

int main(int argc, char* argv[]) 
{
    // program inputs 
    std::fstream file {};
    std::string filepath {};
    std::unordered_map<char, int> char_counts {};
    
    // process args
    bool ok {};
    ok = process_arguments(file, argc, argv);
    if (!ok)
    {
        return 1;
    }

    ok = count_chars(file, char_counts);
    if (!ok) 
    {
        std::cerr << "Failed to read file.\n";
        return 1;
    }

    return 0;
}

