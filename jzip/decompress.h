# pragma once

#include <iostream>
#include <fstream>

bool decompress_file(std::ifstream& compressed_file, std::ofstream& output_file);