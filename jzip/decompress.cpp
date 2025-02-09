#include <iostream>
#include <fstream>
#include <unordered_map>
#include <string>

#include "utils.h"


bool read_code_by_bits_from_file(std::ifstream& infile, uint8_t bit_count, std::string& code_string)
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
            code_string += ((current_byte >> i) & 1) ? '1' : '0';
            --remaining_bits;
        }
    }

    if (infile.bad())
    {
        return false;
    }

    return true;
}

bool read_header_from_compressed_file(std::ifstream& infile, std::unordered_map<char, std::string>& prefix_table)
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
        ok = read_code_by_bits_from_file(infile, prefix_code_size, prefix_code);
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

bool decode_body_w_reverse_prefix_table(const std::string& encoded_body, std::unordered_map<std::string, char> reverse_prefix_table, std::string& decoded_body)
{
    std::string current_code {};
    for (auto& ch : encoded_body)
    {
        current_code += ch;
        auto it { reverse_prefix_table.find(current_code) };
        if (it == reverse_prefix_table.end()) { continue; }
        decoded_body += it->second;
        current_code = "";
    }

    return true;
}

// TODO: clean the handling of chunk_size. 
bool read_encoded_body_from_compressed_file(std::ifstream& infile, std::string& encoded_body)
{
    bool ok {};

    // don't want to read the trailing 0s packed in with the last byte.
    uint8_t trailing_bits {};
    infile.read(reinterpret_cast<char*>(&trailing_bits), sizeof(trailing_bits));

    // get the byte size of the body
    std::streampos body_start { infile.tellg() };
    infile.seekg(0, std::ios::end);
    std::streampos body_end { infile.tellg() };
    infile.seekg(body_start, std::ios::beg);

    // get the bit size of the body, allow narrowing conversion. Dont forget to treat 
    // for trailing bits here. 
    int remaining_bits = (body_end - body_start) * 8 - trailing_bits;

    // read the file chunk by chunk. This chunk must be a multiple of 8, otherwise we 
    // end up skipping bits. We use an int type here because we want to compare with 
    // another int type later to get the minimum. 
    int chunk_size { 31 * 8 };

    std::string encoded_body {};
    std::string decoded_body {};

    while (remaining_bits > 0)
    {
        uint8_t bits_to_read = std::min(chunk_size, remaining_bits);
        std::string bit_string {};

        ok = read_code_by_bits_from_file(infile, bits_to_read, bit_string);
        if (!ok)
        {
            return false;
        }

        encoded_body += bit_string;
        remaining_bits -= bits_to_read;
    }

    ok = infile.bad();
    if (!ok)
    {
        return false;
    }

    return true;
}

// would it be faster to use the reversed map, versus searching the prefix code tree?
// both are linear but tree construction from header is cumbersome.
bool read_decoded_body_from_compressed_file(std::ifstream& infile, const std::unordered_map<char, std::string>& prefix_table, std::string& decoded_body)
{
    std::unordered_map<std::string, char> reverse_prefix_table { reverse_map(prefix_table) };
    std::string encoded_body {};
    bool ok {};

    ok = read_encoded_body_from_compressed_file(infile, encoded_body);
    if (!ok)
    {
        return false;
    }

    ok = decode_body_w_reverse_prefix_table(encoded_body, reverse_prefix_table, decoded_body);
    if (!ok)
    {
        return false;
    }
    
    return true;
}

// writes decompressed file to output file.
bool decompress_file(std::ifstream& compressed_file, std::ofstream& output_file)
{
    std::unordered_map<char, std::string> prefix_table {};
    std::string decoded_body {};
    bool ok {};

    ok = read_header_from_compressed_file(compressed_file, prefix_table);
    if (!ok) 
    {
        return false;
    }

    ok = read_decoded_body_from_compressed_file(compressed_file, prefix_table, decoded_body);
    if (!ok) 
    {
        return false;
    }

    output_file << decoded_body;

    ok = compressed_file.bad() || output_file.bad();
    if (!ok)
    {
        return false;
    }

    return true;
}