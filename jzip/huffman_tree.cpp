#include <iostream>
#include <memory>
#include <map>
#include <queue>

// TODO: combine HuffBaseNode, HuffLeafNode, HuffInternalNode to a single class. 

// interface for node 
class HuffBaseNode
{
public:
    // supply defaults so that we can use the interface in std::make_unique.
    virtual bool is_leaf() const
    {
        return true;
    }

    virtual int get_weight() const
    {
        return 0;
    };

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

    int get_weight() const override
    {
        return m_weight;
    }

    char get_char() const
    {
        return m_char;
    }

    bool is_leaf() const override
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
    HuffInternalNode(std::unique_ptr<HuffBaseNode> left, std::unique_ptr<HuffBaseNode> right, int weight)
        : m_weight { weight }
        // move ownership of left and right nodes to the new internal node instance instead of copying. 
        , m_left { std::move(left) }
        , m_right { std::move(right) }
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
    // default
    HuffmanTree()
    {
    }

    // Leaf node constructor 
    HuffmanTree(char char_, int weight)
        : m_root { std::make_unique<HuffLeafNode>(char_, weight) }
    {
    }

    // Internal node constructor 
    HuffmanTree(std::unique_ptr<HuffBaseNode> left, std::unique_ptr<HuffBaseNode> right, int weight)
        : m_root { std::make_unique<HuffInternalNode>(std::move(left), std::move(right), weight) }
    {
    }

    // Move constructor
    HuffmanTree(HuffmanTree&& other) noexcept 
        : m_root(std::move(other.m_root))
    {
    }

    // Move assignment operator 
    HuffmanTree& operator=(HuffmanTree&& other) noexcept
    {
        if (this != &other)
        {
            m_root = std::move(other.m_root);
        }

        return *this;
    }

    const HuffBaseNode& get_root() const
    {
        return *m_root;
    }

    const int get_weight() const 
    {
        return m_root ? m_root->get_weight() : 0;
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

HuffmanTree build_tree(const std::map<char, int>& char_counts)
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
        // temporarily remove const to move ownership. This is safe becuase the item is quickly popped from the queue.
        HuffmanTree first { std::move(const_cast<HuffmanTree&>(min_heap.top())) };
        min_heap.pop();
        HuffmanTree second { std::move(const_cast<HuffmanTree&>(min_heap.top())) };
        min_heap.pop();
        
        HuffmanTree combined 
        {
            std::make_unique<HuffBaseNode>(std::move(first.get_root())),
            std::make_unique<HuffBaseNode>(std::move(second.get_root())),
            first.get_weight() + second.get_weight()
        };
        min_heap.push(std::move(combined));
    }

    HuffmanTree combined { std::move(const_cast<HuffmanTree&>(min_heap.top())) };
    min_heap.pop();

    return combined;
}

int main()
{
    std::map<char, int> char_counts 
    {
        { 'a', 5 },
        { 'b', 9 },
        { 'c', 3 }
    };

    HuffmanTree tree { build_tree(char_counts) };
    std::cout << tree.get_weight() << std::endl;
    return 0;
}