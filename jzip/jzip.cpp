#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <filesystem>
#include <unistd.h>
#include <unordered_map>
#include <bitset>

#include "huffman_tree.h"

const char* PROGRAM_NAME;

const std::string HEADER_DELIMITER { "[[ENDOFHEADER]]" };

void print_usage(std::ostream& stream)
{
    stream << "jzip compresses files or expands them depending on the filetype passed.\n"
           << "Usage: " << PROGRAM_NAME << "<-oh>" <<" <filepath>\n"
           << "\t-o the output filename.\n"
           << "\t-h display this usage information.\n";
}

bool process_arguments(std::ifstream& infile, std::string& outfile_name, int argc, char* argv[])
{
    PROGRAM_NAME = argv[0];
    int opt {};
    const char* opt_flags { "ho:" };
    
    while ((opt = getopt(argc, argv, opt_flags)) != -1)
    {
        switch (opt)
        {
        case 'h':
            print_usage(std::cout);
            std::exit(0);
        case 'o':
            outfile_name = optarg;
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

        infile.open(filepath, std::ios::in | std::ios::binary);

        if (!infile.is_open())
        {
            std::cerr << "Error: file " << filepath << " could not be opened.\n";
            return false;
        }

        // if output file name is not provided, use input file name
        outfile_name = outfile_name != "" ? outfile_name + ".jzip" : filepath + ".jzip";

        // ensure output file does not already exist.
        std::filesystem::path outfile_path { outfile_name };
        if (std::filesystem::exists(outfile_path))
        {
            std::cerr << "Error: the file " << outfile_path << " already exists. Delete this file before proceeding\n";
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

bool count_chars(std::ifstream& file, std::unordered_map<char, int>& char_counts) 
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

bool create_tree(const std::unordered_map<char, int>& char_counts, HuffmanTree& tree)
{
    try
    {
        tree = build_tree(char_counts);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        return false;
    }
    return true;
}

bool create_prefix_table(const HuffmanTree& tree, std::unordered_map<char, std::string>& table)
{
    try 
    {
        table = build_prefix_code_table(tree);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << "\n";
        return false;
    }
    return true;
}

bool write_header(const std::unordered_map<char, std::string>& prefix_table, std::ofstream& outfile)
{
    for (const auto& kv : prefix_table)
    {
        // store as hex to simplify treatment of special chars like new line. 
        outfile << std::hex << static_cast<int>(static_cast<unsigned char>(kv.first)) << " " << kv.second << "\n";
    }
    outfile << HEADER_DELIMITER << "\n";
    
    if (outfile.bad())
    {
        return false;
    }

    return true;
}

bool read_header(std::ifstream& file, std::unordered_map<char, std::string>& prefix_table)
{
    std::string line {};
    std::string hex_char {};
    char char_ {};
    std::string prefix_code {};
    while (std::getline(file, line) && line != HEADER_DELIMITER)
    {
        try 
        {
            std::istringstream iss { line };
            iss >> hex_char;
            iss >> prefix_code;
            char_ = static_cast<char>(std::stoi(hex_char, nullptr, 16));
            std::cout << char_ << " " << prefix_code << "\n";
            prefix_table[char_] = prefix_code;
        }
        catch(std::runtime_error& e)
        {
            std::cerr << e.what() << "\n";
            return false;
        }
    }

    return  true;
}

std::vector<uint8_t> pack_bits_to_bytes(const std::string& bits) {
    std::vector<uint8_t> byte_stream;
    uint8_t current_byte = 0;
    int bit_count = 0;

    for (char bit : bits) {
        if (bit == '1') {
            current_byte |= (1 << (7 - bit_count));  // Set the bit at the correct position
        }
        bit_count++;

        if (bit_count == 8) {  // Once we fill a byte, push it to the vector
            byte_stream.push_back(current_byte);
            current_byte = 0;  // Reset the byte for the next group of bits
            bit_count = 0;  // Reset bit counter
        }
    }

    // If there are any remaining bits (less than 8 bits left)
    if (bit_count > 0) {
        byte_stream.push_back(current_byte);  // Add the remaining bits as the final byte
    }

    return byte_stream;
}

bool write_body(const std::unordered_map<char, std::string>& prefix_table, std::ifstream& infile, std::ofstream& outfile)
{
    // go back to the top of the file to read. 
    infile.clear();
    infile.seekg(0, std::ios::beg);

    char c {};
    std::string code {};
    std::string bitstream {};
    while (infile.get(c))
    {
        auto it = prefix_table.find(c);
        if (it != prefix_table.end())
        {
            code = it->second;
        }
        else 
        {
            std::cerr << "unknown character, file has been corrupted\n";
            return false;
        }

        // translate into bitset
        bitstream += code;
    }

    std::vector<uint8_t> byte_stream = pack_bits_to_bytes(bitstream);
    outfile.write(reinterpret_cast<const char*>(byte_stream.data()), byte_stream.size());

    return true;
}

int main(int argc, char* argv[]) 
{
    // program inputs 
    std::ifstream infile {};
    std::string outfile_name {};
    std::string filepath {};
    
    // process args
    bool ok {};
    ok = process_arguments(infile, outfile_name, argc, argv);
    if (!ok)
    {
        return 1;
    }

    std::unordered_map<char, int> char_counts {};
    ok = count_chars(infile, char_counts);
    if (!ok) 
    {
        std::cerr << "Failed to read file.\n";
        return 1;
    }

    HuffmanTree tree {};
    ok = create_tree(char_counts, tree);
    if (!ok)
    {
        std::cerr << "Failed to build huffman tree.\n";
        return 1;
    }

    std::unordered_map<char, std::string> prefix_table {};
    ok = create_prefix_table(tree, prefix_table);
    if (!ok) 
    {
        std::cerr << "Failed to build prefix code table.\n";
        return 1;
    }

    std::ofstream outfile { outfile_name };
    ok = write_header(prefix_table, outfile);
    if (!ok) 
    {
        std::cerr << "Failed to write header to compressed file.\n";
        return 1;
    }

    ok = write_body(prefix_table, infile, outfile);
    if (!ok)
    {
        std::cerr << "Failed to write body to compressed file.\n";
        return 1;
    }

    outfile.close();

    std::ifstream test { "../test.txt.jzip", std::ios::in | std::ios::binary };
    std::unordered_map<char, std::string> testt {};

    ok = read_header(test, testt);
    if (!ok)
    {
        return 1;
    }


    return 0;
}

