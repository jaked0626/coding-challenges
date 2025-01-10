#pragma once

#include <memory>
#include <map>
#include <queue>

class HuffmanTreeNode 
{
private:
    int m_weight {};
    std::shared_ptr<HuffmanTreeNode> m_left {};
    std::shared_ptr<HuffmanTreeNode> m_right {};
    char m_char {};

public:
    HuffmanTreeNode(){};
    HuffmanTreeNode(char char_, int weight); // leaf node 
    HuffmanTreeNode(std::shared_ptr<HuffmanTreeNode> left, std::shared_ptr<HuffmanTreeNode> right); // internal node

    int get_weight() const;
    char get_char() const;
    std::shared_ptr<HuffmanTreeNode> get_left() const;
    std::shared_ptr<HuffmanTreeNode> get_right() const;
    bool is_leaf() const;
};

class HuffmanTree
{
private:
    std::shared_ptr<HuffmanTreeNode> m_root {};
    int m_weight {};

public:
    HuffmanTree(){};
    HuffmanTree(char char_, int weight);
    HuffmanTree(std::shared_ptr<HuffmanTreeNode> left, std::shared_ptr<HuffmanTreeNode> right);

    std::shared_ptr<HuffmanTreeNode> get_root() const;
    int get_weight() const;

    bool operator< (const HuffmanTree& other);
    bool operator> (const HuffmanTree& other);
    bool operator== (const HuffmanTree& other);
};

HuffmanTree build_tree(const std::unordered_map<char, int>& char_counts);
std::unordered_map<char, std::string> build_prefix_code_table(const HuffmanTree& tree);
void build_prefix_code_table_r(const HuffmanTreeNode& node, std::unordered_map<char, std::string>& table, std::string& prefix);