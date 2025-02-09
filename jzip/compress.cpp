#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

#include "huffman.h"
#include "compress.h"

bool count_chars_in_file(std::ifstream& infile, std::unordered_map<char, int>& char_counts) 
{
    // start from the beginning of the file
    infile.clear();
    infile.seekg(0, std::ios::beg);
    
    char ch {};
    while (infile.get(ch))
    {
        ++char_counts[ch];
    }
    if (infile.eof())
    {
        infile.clear();
        infile.seekg(0, std::ios::beg);
    }
    else
    {
        return false;
    }

    return true;
}

bool build_tree_from_file(std::ifstream& infile, HuffmanTree& tree)
{
    std::unordered_map<char, int> char_counts {};
    bool ok {};

    ok = count_chars_in_file(infile, char_counts);
    if (!ok)
    {
        std::cerr << "Failed to process characters in provided file.\n";
        return false;
    }

    tree = build_tree(char_counts);

    return true;
}

bool build_prefix_code_table_from_file(std::ifstream& infile, std::unordered_map<char, std::string>& prefix_code_table)
{
    bool ok {};
    HuffmanTree tree {};

    ok = build_tree_from_file(infile, tree);
    if (!ok)
    {
        std::cerr << "Failed to build tree\n";
        return false;
    }

    prefix_code_table = build_prefix_code_table(tree);

    return true;
}

bool write_code_to_file(std::ofstream& outfile, const std::string& code)
{
    uint8_t current_byte { 0 };
    int bit_count { 0 };

    for (char bit : code)
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

    // left over bits are packed into a byte. 
    if (bit_count > 0)
    {
        outfile.put(current_byte);
    }

    if (outfile.bad())
    {
        return false;
    }

    return true;
}

bool write_compressed_header_to_file(std::ofstream& outfile, const std::unordered_map<char, std::string>& prefix_table)
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
        ok = write_code_to_file(outfile, prefix_code);
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

bool write_compressed_body_to_file(std::ifstream& infile, std::ofstream& outfile, const std::unordered_map<char, std::string>& prefix_table)
{
    // go to the top of the source infile to read the whole file.
    infile.clear();
    infile.seekg(0, std::ios::beg);

    char ch {};
    std::string prefix_code {};
    std::string prefix_codes { "" };
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
            std::cerr << "Source file has been corrupted\n";
            return false;
        }

        // add code to bit_string and write it all to the file as bits in the end.
        prefix_codes += prefix_code;
    }

    // last byte that we write will pack trailing 0s for bits. We don't want to 
    // accidentally read these when decompressing. 
    // double modulo to deal with the case that the size of bit_string is a multiple of 8. 
    // copy intialize to allow narrowing conversion.
    uint8_t trailing_bits = (8 - (prefix_codes.size() % 8)) % 8;

    // we write the number of trailing_bits first so that we know to ignore these 
    // bits when decompressing. 
    outfile.write(reinterpret_cast<const char*>(&trailing_bits), sizeof(trailing_bits));

    ok = write_code_to_file(outfile, prefix_codes);
    if (!ok)
    {
        std::cerr << "Failed to write to file\n";
        return false;
    }

    if (infile.bad() || outfile.bad())
    {
        std::cerr << "Failed to write to file\n";
        return false;
    }

    return true;
}

bool compress_file(std::ifstream& infile, std::ofstream& outfile)
{
    std::unordered_map<char, std::string> prefix_code_table {};
    bool ok {};

    ok = build_prefix_code_table_from_file(infile, prefix_code_table);
    if (!ok)
    {
        return false;
    }

    ok = write_compressed_header_to_file(outfile, prefix_code_table);
    if (!ok)
    {
        return false;
    }

    ok = write_compressed_body_to_file(infile, outfile, prefix_code_table);
    if (!ok)
    {
        return false;
    }

    return true;
}