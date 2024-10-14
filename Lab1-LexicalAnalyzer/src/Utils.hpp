#include<unordered_set>
#include<unordered_map>
#include<vector>
#include<cstdint>

template <bool cond, typename A, typename B>
struct meta_elif {
    using type = A;
};

template <typename A, typename B>
struct meta_elif<false, A, B> {
    using type = B;
};

template <typename A, typename B>
struct unordered_pair : std::pair<A, B> {
    unordered_pair(A a, B b) : std::pair<A, B>(a, b) {};
    
    template <template <typename, typename> class PAIR>
    bool operator== (const PAIR<A, B>& other) {
        return this->first == other.first && this->second == other.second;
    }
    template <template <typename, typename> class PAIR>
    bool operator== (const PAIR<B, A>& other) {
        return this->second == other.first && this->first == other.second;
    }
};

template <typename A>
struct unordered_pair<A, A> : std::pair<A, A> {
    unordered_pair(A a, A b) : std::pair<A, A>(a, b) {};

    template <template <typename, typename> class PAIR>
    bool operator== (const PAIR<A, A>& other) {
        return this->first == other.first && this->second == other.second
            || this->second == other.first && this->first == other.second;
    }
};

template<typename T>
struct Initializer {
    T init;
    Initializer(T init) : init(std::move(init)) {}
    Initializer(Initializer<T>&& init) : init(std::move(init.init)) {}
    Initializer() {}
};

template<typename T>
constexpr bool predicate_true (T x) {
    return true;
}

template<typename T>
constexpr bool predicate_false (T x) {
    return false;
}

template<typename T>
void make_set_union(std::unordered_set<T>& s1, const std::unordered_set<T>& s2) {
    for (const T& el : s2) 
        s1.insert(el);
}

template<typename T>
std::unordered_set<T> set_union_of(const std::unordered_set<T>& s1, const std::unordered_set<T>& s2) {
    std::unordered_set<T> rez = s1;
    make_set_union(rez, s2);
    return rez;
}

// namespace redacted {

// template<typename N, typename E, bool DIRECTED = false>
// struct Graph {
//     using ID = uint32_t;

//     struct Node;
//     struct Edge : meta_elif<DIRECTED, std::pair<ID, ID>, unordered_pair<ID, ID>>::type {};

//     template<typename T>
//     using Set = std::unordered_set<T>;
//     using EdgeMap = std::unordered_map<Edge, E>;
//     using NodeInit = Initializer<N>;

//     struct ILinkable {
//         virtual void onLink(Node* n, const E& e) = 0; 
//     };

// private:
//     std::vector<Node> _nodes;
//     EdgeMap _edges;

// public:
//     const std::vector<Node>& nodes() {
//         return _nodes;
//     } 
//     E& edge(Node* n1, Node* n2) {
//         return _edges[{n1->id, n2->id}];
//     } 

//     Node &operator[] (uint32_t i) {
//         return _nodes[i];
//     }

//     Node* make_node(NodeInit init = NodeInit()) {
//         return _nodes.emplace(this, std::move(init));
//     }

//     void make_edge (Node* n1, Node* n2, const E& edge) {
//         _edges[{n1->id, n2->id}] = edge;
//     }
    
//     struct Node {
//     friend N;

//         N content;
//         const ID id;

//     private:
//         Graph* const parent;

//         Node(Graph* parent, NodeInit init = NodeInit()) 
//             : parent(parent), id(parent->_nodes.size()), content(std::move(init)) {}

//     public: 
//         Node* add(const E& edge, NodeInit init = NodeInit()) {
//             Node* new_node = parent->make_node(init);
//             this->link(edge, new_node);
//             return new_node;
//         }
//         void link(const E& edge, Node* other) {
//             if (std::is_base_of_v<ILinkable, N>)
//                 content.onLink(other, edge);
//             if (std::is_base_of_v<ILinkable, N> && !DIRECTED)
//                 other->content.onLink(this, edge);
//             parent->make_edge(this, other, edge);
//         }
//     };
// };

// template<typename N, typename E, bool DIRECTED = false>
// struct SimpleGraph {
//     struct node_info;

//     using BaseGraph = Graph<node_info, E, DIRECTED>;
//     using ID = typename BaseGraph::ID;
//     using Node = typename BaseGraph::Node;
//     using NodeInit = Initializer<N>;

//     template<typename T>
//     using Set = std::unordered_set<T>;
    
//     struct node_info : BaseGraph::ILinkable {
//         SimpleGraph* const parent;
//     private:
//         N value;
//         Set<ID> neighbors;

//         node_info(SimpleGraph* parent, NodeInit value = NodeInit()) : parent(parent), value(std::move(value)) {}

//         void onLink (Node* node, const E& edge) {
//             neighbors.insert(node->id);
//         }
//     };

// private:
//     BaseGraph graph;

// public:
//     SimpleGraph() {}

//     Node* make_node(NodeInit init = NodeInit()) {
//         return graph.make_node(this, init);
//     }

//     void make_edge(Node* n1, Node* n2, const E& edge) {
//         return graph.make_edge(n1, n2, edge);
//     }
// };

// }