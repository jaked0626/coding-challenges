#include <memory>
#include <map>
#include <queue>
#include <iostream>

#include "huffman_tree.h"

/* HuffmanTreeNode */

// leaf node constructor 
HuffmanTreeNode::HuffmanTreeNode(char char_, int weight)
    : m_char { char_ }
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
HuffmanTree::HuffmanTree(char char_, int weight)
    : m_root { std::make_shared<HuffmanTreeNode>(char_, weight) }
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

    for (const auto& kv : char_counts)
    {
        char char_ { kv.first };
        int weight { kv.second };
        HuffmanTree tree { char_, weight };
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

#ifdef TEST_HUFFMAN_TREE
int main()
{
    std::unordered_map<char, int> char_counts 
    {
        { 'a', 5 },
        { 'b', 9 },
        { 'c', 3 },
        { 'z', 50 },
        { 'e', 5 }
    };

    HuffmanTree tree { build_tree(char_counts) };
    std::cout << tree.get_weight() << std::endl;
    return 0;
}
#endif // TEST_HUFFMAN_TREE