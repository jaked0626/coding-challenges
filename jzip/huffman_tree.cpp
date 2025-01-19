#include <memory>
#include <map>
#include <queue>
#include <iostream>

#include "huffman_tree.h"

/* HuffmanTreeNode */

// leaf node constructor 
HuffmanTreeNode::HuffmanTreeNode(char ch, int weight)
    : m_char { ch }
    , m_weight { weight }
{
}

// internal node constructor
HuffmanTreeNode::HuffmanTreeNode(std::shared_ptr<HuffmanTreeNode> left, std::shared_ptr<HuffmanTreeNode> right)
    : m_left { left }
    , m_right { right }
{
    m_weight = m_left->get_weight() + m_right->get_weight();
}

int HuffmanTreeNode::get_weight() const
{
    return m_weight;
}

char HuffmanTreeNode::get_char() const
{
    return m_char ? m_char : '\0';
}

bool HuffmanTreeNode::is_leaf() const
{
    return (m_left == nullptr) && (m_right == nullptr);
}

std::shared_ptr<HuffmanTreeNode> HuffmanTreeNode::get_left() const
{
    return m_left;
}

std::shared_ptr<HuffmanTreeNode> HuffmanTreeNode::get_right() const
{
    return m_right;
}

/* Huffman Tree */

// initialize with leaf node
HuffmanTree::HuffmanTree(char ch, int weight)
    : m_root { std::make_shared<HuffmanTreeNode>(ch, weight) }
    , m_weight { weight }
{
}

// initialize with internal node 
HuffmanTree::HuffmanTree(std::shared_ptr<HuffmanTreeNode> left, std::shared_ptr<HuffmanTreeNode> right)
    : m_root { std::make_shared<HuffmanTreeNode>(left, right) }
{
    m_weight = m_root->get_weight();
}

std::shared_ptr<HuffmanTreeNode> HuffmanTree::get_root() const
{
    return m_root;
}

int HuffmanTree::get_weight() const 
{
    return m_root ? m_root->get_weight() : 0;
}

bool HuffmanTree::operator< (const HuffmanTree& other)
{
    return get_weight() < other.get_weight();
}

bool HuffmanTree::operator> (const HuffmanTree& other)
{
    return get_weight() > other.get_weight();
}

bool HuffmanTree::operator== (const HuffmanTree& other)
{
    return get_weight() == other.get_weight();
}

HuffmanTree build_tree(const std::unordered_map<char, int>& char_counts)
{
    // enqueue huffman trees
    std::priority_queue<HuffmanTree, std::vector<HuffmanTree>, std::greater<>> min_heap {};

    for (const auto& [ch, weight] : char_counts)
    {
        HuffmanTree tree { ch, weight };
        min_heap.push(std::move(tree));
    }

    // combine huffman trees
    while (min_heap.size() > 1)
    {
        HuffmanTree first { min_heap.top() };
        min_heap.pop();
        HuffmanTree second { min_heap.top() };
        min_heap.pop();
        HuffmanTree combined { first.get_root(), second.get_root() };
        min_heap.push(combined);
    }

    return min_heap.top();
}

std::unordered_map<char, std::string> build_prefix_code_table(const HuffmanTree& tree)
{
    std::unordered_map<char, std::string> table {};
    std::string prefix {};
    if (auto root { tree.get_root() }; root != nullptr)
    {
        build_prefix_code_table_r(*root, table, prefix);
    }

    return table;
}

void build_prefix_code_table_r(const HuffmanTreeNode& node, std::unordered_map<char, std::string>& table, std::string& prefix)
{
    if (node.is_leaf())
    {
        table[node.get_char()] = prefix;
    }
    else
    {
        if (node.get_left() != nullptr)
        {
            prefix.push_back('0');
            build_prefix_code_table_r(*node.get_left(), table, prefix);
            prefix.pop_back();
        }
        if (node.get_right() != nullptr)
        {
            prefix.push_back('1');
            build_prefix_code_table_r(*node.get_right(), table, prefix);
            prefix.pop_back();
        }
    }

    return;
}

char get_char_from_code(const std::string& prefix_code, const HuffmanTree& tree)
{
    std::shared_ptr<HuffmanTreeNode> current_node { tree.get_root() };
    for (auto& ch : prefix_code)
    {
        if (ch == '0')
        {
            current_node = current_node->get_left();
        }
        else if (ch == '1')
        {
            current_node = current_node->get_right();
        }
    }

    return current_node->get_char();
}

std::string get_string_from_codes(const std::string& prefix_codes, const HuffmanTree& tree)
{
    std::string output {};
    std::shared_ptr<HuffmanTreeNode> current_node { tree.get_root() };
    for (auto& ch : prefix_codes)
    {
        if (ch == '0')
        {
            current_node = current_node->get_left();
        }
        else if (ch == '1')
        {
            current_node = current_node->get_right();
        }

        if (current_node->is_leaf())
        {
            output += current_node->get_char();
            current_node = tree.get_root();
        }
    }
}


#ifdef TEST_HUFFMAN_TREE
int main()
{
    std::unordered_map<char, int> char_counts 
    {
        { 'a', 5 },
        { 'b', 9 },
        { 'c', 3 },
        { 'z', 50 },
        { 'e', 6 }
    };

    HuffmanTree tree { build_tree(char_counts) };
    std::unordered_map<char, std::string> table { build_prefix_code_table(tree) };

    for (const auto& kv : table) 
    {
        std::cout << kv.first << " : " << kv.second << "\n";
    }

    return 0;
}
#endif // TEST_HUFFMAN_TREE