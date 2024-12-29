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
    bool is_count_chars {};
    bool is_standard_input { false };
};

void print_usage(std::ostream& stream)
{
    stream << "Usage: " << program_name << " <-clwm> <filepath>\n"
           << "\t-h display this usage information.\n"
           << "\t-c count the number of bytes for a given file.\n"
           << "\t-l count the number of lines in a given file.\n"
           << "\t-w count the number of words in a given file.\n"
           << "\t-m count the number of characters in a given file.\n";
}

bool process_arguments(std::ifstream& file, std::string& filepath, Options& opts, int argc, char* argv[])
{
    program_name = argv[0];
    int opt {};
    const char* opt_flags { "hclwm" };
    
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
       case 'm':
            opts.is_count_chars = true;
            break;
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
        filepath = argv[optind];
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
    // otherwise, we use standard input.
    else 
    {
        // create temporary file 
        // TODO: this is unsafe and has undefined behavior if a file of the same 
        // name as the temp file already exists. furthermore, it would require 
        // clean up of the temp file. 
        opts.is_standard_input = true;
        std::ofstream temp_file { ".ccwc.tmp.txt", std::ios::app };
        if (!temp_file || !temp_file.is_open()) 
        {
            std::cerr << "Error: failed to create temporary file";
            return 1;
        }

        std::string line {};
        while (std::getline(std::cin, line))
        {
            temp_file << line << "\n";
        }
        temp_file.close();

        file.open(".ccwc.tmp.txt", std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            std::cerr << "Error: file " << filepath << " could not be opened.\n";
            return false;
        }
    }

    return true;
}

bool add_byte_count(std::ifstream& file, std::stringstream& ss)
{
    int num_bytes { 0 };
    char c {};

    while (file.get(c))
    {
        ++num_bytes;
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

    ss << "\t" << num_bytes;
    
    return true;
}

bool add_line_count(std::ifstream& file, std::stringstream& ss)
{
    int num_lines { 0 };
    std::string line {};
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
    std::string word {};
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

bool add_char_count(std::ifstream& file, std::stringstream& ss)
{
    int num_chars { 0 };
    char c {};

    while (file.get(c))
    {
        // presupposes utf-8 file encoding. In utf-8, characters are encoded in 
        // 1~4 bytes, and continuation bytes of multibyte characters begin with 
        // 10xxxxxx. We want to skip such continuations. 
        // First we want to make sure they are not singlebyte characters. 
        // 128 = 01000000, and anything above that is a multibyte character in utf-8.
        // Then, extract the first two bits to see if they match the continuation 
        // character. 
        if ((unsigned char) c >= 128 && ((unsigned char) c & 0b11000000) == 0b10000000)
        {
            continue;
        }
        ++num_chars;
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

    ss << "\t" << num_chars;
    
    return true;
}

int main(int argc, char* argv[]) 
{
    // program inputs 
    std::ifstream file {};
    std::string filepath {};
    Options opts {};

    // output 
    std::stringstream out {};
    
    // process args
    bool ok {};
    ok = process_arguments(file, filepath, opts, argc, argv);
    if (!ok)
    {
        return 1;
    }

    
    // support for default option 
    if (!(opts.is_count_bytes || opts.is_count_lines || opts.is_count_words || opts.is_count_chars))
    {
        opts.is_count_bytes = true;
        opts.is_count_lines = true;
        opts.is_count_words = true;
    }

    if (opts.is_count_bytes && !add_byte_count(file, out)) 
    {
        std::cerr << "Error: could not read bytes of file.\n";
    }

    if (opts.is_count_lines && !add_line_count(file, out))
    {
        std::cerr << "Error: could not read lines of file.\n";
    }

    if (opts.is_count_words && !add_word_count(file, out))
    {
        std::cerr << "Error: could not read words of file.\n";
    }

    if (opts.is_count_chars && !add_char_count(file, out))
    {
        std::cerr << "Error: could not read chars of file.\n";
    }

    out << "\t" << filepath;

    std::cout << out.str() << std::endl;

    if (opts.is_standard_input)
    {
        if (std::remove(".ccwc.tmp.txt") != 0)
        {
            std::cerr << "Error: failed to remove temp file .ccwc.tmp.txt" << "\n";
            return 1;
        }
    }

    return 0;
}

