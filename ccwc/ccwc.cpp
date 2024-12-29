#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <unistd.h>

const char* program_name;

struct Options
{
    bool is_count_bytes {};
    bool is_count_lines {};
    bool is_count_words {};
};

void print_usage(std::ostream& stream)
{
    stream << "Usage: " << program_name << " <-clw> <filepath>\n"
        << "\t-h display this usage information.\n"
        << "\t-c count the number of bytes for a given file.\n"
        << "\t-l count the number of lines in a given file.\n"
        << "\t-w count the number of words in a given file.\n";
}

bool process_arguments(std::string& filepath, Options& opts, int argc, char* argv[])
{
    program_name = argv[0];
    int opt {};
    const char* opt_flags { "hclw" };
    
    while ((opt = getopt(argc, argv, opt_flags)) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(std::cout);
            std::exit(0);
        case 'c':
            opts.is_count_bytes = true;
            break;
        case 'l':
            opts.is_count_lines = true;
            break;
       case 'w':
            opts.is_count_words = true;
            break;
        case '?':
            print_usage(std::cerr);
            return false;
        default:
            return false;
        }
    }

    if (optind < argc) 
    {
        filepath = argv[optind];
    }
    else 
    {
        std::cerr << "Error: A filepath must be provided.\n";
        print_usage(std::cerr);
        return false;
    }

    return true;
}

bool add_byte_count(const std::filesystem::path& p, std::stringstream& ss)
{
    try 
    {
        std::uintmax_t num_bytes { std::filesystem::file_size(p) };
        ss << "\t" << num_bytes;
    }
    catch (const std::filesystem::filesystem_error& e)
    {
        return false;
    }
    return true;
}

bool add_line_count(std::ifstream& file, std::stringstream& ss)
{
    int num_lines { 0 };
    std::string line;
    while (std::getline(file, line))
    {
        ++num_lines;
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

    ss << "\t" << num_lines;
    
    return true;
}

bool add_word_count(std::ifstream& file, std::stringstream& ss)
{
    int num_words { 0 };
    std::string word;
    while (file >> word)
    {
        ++num_words;
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

    ss << "\t" << num_words;
    
    return true;
}

int main(int argc, char* argv[]) 
{
    // program options 
    std::string filepath {};
    Options opts {};

    // output 
    std::stringstream out {};
    
    // process args 
    bool ok {};
    ok = process_arguments(filepath, opts, argc, argv);
    if (!ok) 
    {
        return 1;
    }

    std::filesystem::path file_system_path { filepath };
    if (!std::filesystem::exists(file_system_path))
    {
        std::cerr << "Error: file " << filepath << " does not exist.\n";
        return 1;
    }
    if (!std::filesystem::is_regular_file(file_system_path))
    {
        std::cerr << "Error: file " << filepath << " is not a regular file.\n";
        return 1;
    }

    std::ifstream file { filepath, std::ios::in | std::ios::binary };
    if (!file.is_open()) 
    {
        std::cerr << "Error: file " << filepath << " could not be opened.\n";
        return 1;
    }
    
    // support for default option 
    if (!(opts.is_count_bytes || opts.is_count_lines || opts.is_count_words))
    {
        opts.is_count_bytes = true;
        opts.is_count_lines = true;
        opts.is_count_words = true;
    }

    if (opts.is_count_bytes) 
    {
        ok = add_byte_count(filepath, out);
        if (!ok) 
        {
            std::cerr << "Error: could not read bytes of file.\n";
        }
    }

    if (opts.is_count_lines)
    {
        ok = add_line_count(file, out);
        if (!ok)
        {
            std::cerr << "Error: could not read lines of file.\n";
            file.close();
            file.open(filepath, std::ios::in | std::ios::binary);
        }
    }

    if (opts.is_count_words)
    {
        ok = add_word_count(file, out);
        if (!ok)
        {
            std::cerr << "Error: could not read words of file.\n";
            file.close();
            file.open(filepath, std::ios::in | std::ios::binary);
        }
    }

    out << "\t" << filepath;

    std::cout << out.str() << std::endl;

    return 0;
}

