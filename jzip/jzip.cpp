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
    char ch {};
    while (file.get(ch))
    {
        ++char_counts[ch];
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

std::vector<uint8_t> codes_to_bytes(const std::string& code) 
{
    std::vector<uint8_t> byte_stream {};
    uint8_t current_byte { 0 }; 
    int bit_count { 0 };

    for (char bit : code) 
    {
        if (bit == '1')
        {
            current_byte |= (1 << (7 - bit_count)); // adds 1 to the desired position.
        }
        bit_count++;

        if (bit_count == 8)
        {
            byte_stream.push_back(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }

    // take care of the remaining bits and pack them into a single byte. 
    if (bit_count > 0) 
    {
        byte_stream.push_back(current_byte);
    }

    return byte_stream;
}

bool write_bits(std::ofstream& outfile, const std::string& bit_string)
{
    uint8_t current_byte { 0 };
    int bit_count { 0 };

    for (char bit : bit_string)
    {
        if (bit == '1')
        {
            current_byte |= (1 << (7 - bit_count)); // adds 1 to the desired position.
        }
        ++bit_count;

        // flush byte
        if (bit_count == 8)
        {
            outfile.put(current_byte);
            current_byte = 0;
            bit_count = 0;
        }
    }

    // left over bits are packed into a byte
    if (bit_count > 0)
    {
        current_byte <<= (8 - bit_count); // shift the bits and pack them to the left
        outfile.put(current_byte);
    }

    if (outfile.bad())
    {
        return false;
    }

    return true;
}

bool read_code_from_bits(std::ifstream& infile, uint8_t bit_count, std::string& prefix_code)
{
    uint8_t current_byte {};
    int remaining_bits { bit_count };
    int remaining_bits_per_byte {};

    while (remaining_bits > 0)
    {
        infile.read(reinterpret_cast<char*>(&current_byte), 1);

        // we want to read the byte bit by bit from the left most bit. 
        // since we are reading in chunks by bytes, we need to keep track of the 
        // remaining bits so as not to over read.
        remaining_bits_per_byte = std::min(remaining_bits, 8);

        for (int i = 7; i >= 8 - remaining_bits_per_byte; --i)
        {
            prefix_code += ((current_byte >> i) & 1) ? '1' : '0';
            --remaining_bits;
        }
    }

    if (infile.bad())
    {
        return false;
    }

    return true;
}

bool write_header(std::ofstream& outfile, const std::unordered_map<char, std::string>& prefix_table)
{
    // we write the number of elements in the prefix table to the header first to determine 
    // how much of the file is part of the header.
    uint16_t header_size = prefix_table.size(); // copy initialize to allow narrowing conversion.
    outfile.write(reinterpret_cast<const char*>(&header_size), sizeof(header_size));
    bool ok {};

    for (const auto& [ch, prefix_code] : prefix_table)
    {
        // we place the character, the size of the prefix_code, and the prefix_code in that 
        // order, so we know how many bits to read to retrieve the code.
        uint8_t prefix_code_size = prefix_code.size(); // copy initialize to allow narrowing conversion.
        outfile.put(ch);
        outfile.put(prefix_code_size);
        ok = write_bits(outfile, prefix_code);
        if (!ok)
        {
            return false;
        }
    }

    if (outfile.bad())
    {
        return false;
    }

    return true;
}

bool read_header(std::ifstream& infile, std::unordered_map<char, std::string>& prefix_table)
{
    bool ok {};

    // read first 16 bits where we store the number of elements in the prefix_table.
    uint16_t header_size {};
    infile.read(reinterpret_cast<char*>(&header_size), sizeof(header_size));

    for (uint16_t i = 0; i < header_size; ++i)
    {
        char ch {};
        uint8_t prefix_code_size {};
        std::string prefix_code {};

        // read ch, prefix_code_size, and code in that order. ch is 1 byte and 
        // code is number of bits specified in prefix_code_size
        infile.read(&ch, 1);
        infile.read(reinterpret_cast<char*>(&prefix_code_size), 1);
        ok = read_code_from_bits(infile, prefix_code_size, prefix_code);
        if (!ok)
        {
            return false;
        }
        prefix_table[ch] = prefix_code;
    }

    if (infile.bad())
    {
        return false;
    }

    return  true;
}

bool write_body(std::ifstream& infile, std::ofstream& outfile, const std::unordered_map<char, std::string>& prefix_table)
{
    // go back to the top of the source infile to read the whole file.
    infile.clear();
    infile.seekg(0, std::ios::beg);

    char ch {};
    std::string prefix_code {};
    std::string bit_string { "" };
    bool ok {};

    while (infile.get(ch))
    {
        // retrieve prefix code from table. If the char is not in the table, we've fucked up. 
        auto it { prefix_table.find(ch) };
        if (it != prefix_table.end())
        {
            prefix_code = it->second;
        }
        else 
        {
            return false;
        }

        // add code to bit_string and write it all to the file as bits in the end.
        bit_string += prefix_code;
    }

    ok = write_bits(outfile, bit_string);
    if (!ok)
    {
        return false;
    }

    if (infile.bad() || outfile.bad())
    {
        return false;
    }

    return true;
}

template <typename K, typename V>
std::unordered_map<V, K> reverse_map(const std::unordered_map<K, V>& map)
{
    std::unordered_map<K, V> reversed_map {};
    for (auto& [k, v] : map)
    {
        reversed_map[v] = k;
    }

    return reversed_map;
}

// bool read_body(std::ifstream& infile, std::ofstream& outfile, const std::unordered_map<char, std::string>& prefix_table)
// {
//     std::unordered_map<std::string, char> reverse_prefix_table { reverse_map(prefix_table) };

    
//     return true;
// }

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
    ok = write_header(outfile, prefix_table);
    if (!ok) 
    {
        std::cerr << "Failed to write header to compressed file.\n";
        return 1;
    }

    ok = write_body(infile, outfile, prefix_table);
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

    for (auto& [ch, code] : testt)
    {
        std::cout << ch << ":" << code << "\n";
    }

    return 0;
}

