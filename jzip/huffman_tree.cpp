#include <iostream>
#include <memory>
#include <map>
#include <queue>

// interface for node 
class HuffBaseNode
{
public:
    virtual bool is_leaf() const = 0;
    virtual int get_weight() const = 0;

    virtual ~HuffBaseNode() = default;
};

class HuffLeafNode : public HuffBaseNode 
{
private:
    char m_char {};
    int m_weight {};

public: 
    HuffLeafNode(char char_, int weight)
        : m_char { char_ }
        , m_weight { weight }
    {
    }

    int get_weight() const
    {
        return m_weight;
    }

    char get_char() const
    {
        return m_char;
    }

    bool is_leaf() const
    {
        return true;
    }
};

class HuffInternalNode : public HuffBaseNode 
{
private:
    int m_weight {};
    std::unique_ptr<HuffBaseNode> m_left {};
    std::unique_ptr<HuffBaseNode> m_right {};

public:
    HuffInternalNode(HuffBaseNode* left, HuffBaseNode* right, int weight)
        : m_weight { weight }
        , m_left { left } 
        , m_right { right }
    {
    }

    const HuffBaseNode& get_left() const
    {
        return *m_left;
    }

    const HuffBaseNode& get_right() const
    {
        return *m_right;
    }

    int get_weight() const
    {
        return m_weight;
    }

    bool is_leaf() const
    {
        return false;
    }
};

class HuffmanTree 
{
private:
    std::unique_ptr<HuffBaseNode> m_root {};

public:
    HuffmanTree()
    {
    }

    HuffmanTree(char char_, int weight)
        : m_root { &HuffLeafNode(char_, weight) }
    {
    }

    HuffmanTree(HuffBaseNode* left, HuffBaseNode* right, int weight)
        : m_root { &HuffInternalNode(left, right, weight) }
    {
    }

    const HuffBaseNode& get_root() const
    {
        return *m_root;
    }

    const int get_weight() const 
    {
        return m_root->get_weight();
    }

    friend bool operator== (const HuffmanTree& t1, const HuffmanTree& t2)
    {
        return t1.get_weight() == t2.get_weight();
    }

    friend bool operator!= (const HuffmanTree& t1, const HuffmanTree& t2)
    {
        return !(operator==(t1, t2));
    }

    friend bool operator< (const HuffmanTree& t1, const HuffmanTree& t2)
    {
        return t1.get_weight() < t2.get_weight();
    }

    friend bool operator> (const HuffmanTree& t1, const HuffmanTree& t2)
    {
        return operator<(t2, t1);
    }

    friend bool operator<= (const HuffmanTree& t1, const HuffmanTree& t2)
    {
        return !(operator>(t1, t2));
    }

    friend bool operator>= (const HuffmanTree& t1, const HuffmanTree& t2)
    {
        return !(operator<(t1, t2));
    }
};

// HuffmanTree build_tree(std::map<int, char> char_counts)
// {
//     // enqueue huffman trees
//     std::priority_queue<HuffmanTree, std::vector<HuffmanTree>, std::greater<>> min_heap {};

//     for (const auto& kv : char_counts)
//     {
//         int weight { kv.first };
//         char char_ { kv.second };
//         HuffmanTree tree { char_, weight };
//         min_heap.push(tree);
//     }

//     // combine huffman trees
//     HuffmanTree first {};
//     HuffmanTree second {};
//     HuffmanTree combined {};
//     while (min_heap.size() > 1)
//     {
//         first = min_heap.top();
//         min_heap.pop();
//         second = min_heap.top();
//         min_heap.pop();
//     }
// }

