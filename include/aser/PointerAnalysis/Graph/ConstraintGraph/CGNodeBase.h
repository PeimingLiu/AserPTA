//
// Created by peiming on 11/1/19.
//
#ifndef ASER_PTA_CGNODEBASE_H
#define ASER_PTA_CGNODEBASE_H

#include <llvm/ADT/SparseBitVector.h>

#include "aser/PointerAnalysis/Graph/GraphBase/GraphBase.h"

namespace aser {

template <typename ctx>
class CallGraphNode;

template <typename ctx>
class ConstraintGraph;

// forward declartion
using NodeID = uint64_t;

const NodeID NULL_PTR = 0;
const NodeID UNI_PTR = 1;
const NodeID NULL_OBJ = 2;
const NodeID UNI_OBJ = 3;
const NodeID NORMAL_NODE_START_ID = 4;

template <typename Pts>
struct PTSTrait;

template <typename Model>
struct LangModelTrait;

// must start from 0 and increase by one!
enum class Constraints : std::uint8_t {
    load = 0,
    store = 1,
    copy = 2,
    addr_of = 3,
    offset = 4,
};

enum class CGNodeKind : uint8_t {
    PtrNode = 0,
    ObjNode = 1,
    SuperNode = 2,
};

template <typename ctx>
class CGNodeBase {
protected:
    using Self = CGNodeBase<ctx>;
    using super = NodeBase<Constraints, CGNodeBase>;
    using GraphTy = GraphBase<Self, Constraints>;

    const CGNodeKind type;
    const NodeID id;
    const GraphTy *graph;

    Self *superNode; // the super node
    llvm::SparseBitVector<> childNodes;

    using SetTy = llvm::DenseSet<Self *>;
    SetTy succCons[5];  // successor constraints
    SetTy predCons[5];  // predecessor constraints

    inline void setGraph(const GraphTy *g) { this->graph = g; }

    // the same function ptr might used multiple times
    // call void %2418(i8* %2910, i8* nonnull %2453, i8* nonnull %2451) #10
    // call void %2418(i8* nonnull %2454, i8* nonnull %2453, i8* nonnull %2451)
    using IndirectNodeSet = llvm::SmallPtrSet<CallGraphNode<ctx> *, 8>;
    IndirectNodeSet indirectNodes;

    inline CGNodeBase(NodeID id, CGNodeKind type)
        : id(id), type(type), superNode(nullptr), childNodes{}, indirectNodes{} {}

private:
    inline bool insertConstraint(Self *node, Constraints edgeKind) {
        auto src = this->getSuperNode();
        auto dst = node->getSuperNode();

        //assert(!this->hasSuperNode());

        if (src == dst && edgeKind == Constraints::copy ) {
            // self-copy does not have any effect.
            return false;
        }

        auto index = static_cast<std::underlying_type<Constraints>::type>(edgeKind);
        assert(index < 5);  // only 5 kinds of constraints

        // successors
        bool r1 = src->succCons[index].insert(dst).second;
        // predecessors
        bool r2 = dst->predCons[index].insert(src).second;

        assert(r1 == r2);
        return r1;
    }

    // remove one particular edge
    inline void removeConstraint(Self *target, Constraints edgeKind) {
        auto index = static_cast<std::underlying_type<Constraints>::type>(edgeKind);
        assert(index < 5);
        auto iter = succCons[index].find(target);

        if (iter != succCons[index].end()) {
            auto pred_iter = target->predCons[index].find(this);
            assert(pred_iter != target->predCons[index].end());
            succCons[index].erase(iter);
            target->predCons[index].erase(pred_iter);
        }
    }

public:
    // can not be moved and copied
    CGNodeBase(const CGNodeBase<ctx> &) = delete;
    CGNodeBase(CGNodeBase<ctx> &&) = delete;
    CGNodeBase<ctx> &operator=(const CGNodeBase<ctx> &) = delete;
    CGNodeBase<ctx> &operator=(CGNodeBase<ctx> &&) = delete;

    [[nodiscard]]
    inline bool isSpecialNode() const {
        return this->getNodeID() < NORMAL_NODE_START_ID;
    }

    [[nodiscard]]
    inline bool isNullObj() const {
        return this->getNodeID() == NULL_OBJ;
    }

    [[nodiscard]]
    inline bool isUniObj() const {
        return this->getNodeID() == UNI_OBJ;
    }

    [[nodiscard]]
    inline bool isNullPtr() const {
        return this->getNodeID() == NULL_PTR;
    }

    [[nodiscard]]
    inline bool isUniPtr() const {
        return this->getNodeID() == UNI_PTR;
    }

    inline bool isSuperNode() const {
        return !childNodes.empty();
    }

    inline void setSuperNode(Self *node) {
        this->superNode = node;
    }

    inline Self *getSuperNode() {
        Self *node = this;
        while (node->superNode != nullptr) {
            node = node->superNode;
        }
        return node;
    }

    // remove all the edges
    inline void clearConstraints() {
#define CLEAR_CONSTRAINT(TYPE)                                                                        \
    {                                                                                                 \
        constexpr auto index = static_cast<std::underlying_type<Constraints>::type>(Constraints::TYPE);   \
        for (auto it = this->succ_##TYPE##_begin(), ie = this->succ_##TYPE##_end(); it != ie; it++) { \
            Self *target = *it;                                                                       \
            auto iter = target->predCons[index].find(this);                                           \
            assert(iter != target->predCons[index].end());                                            \
            target->predCons[index].erase(iter);                                                      \
        }                                                                                             \
        for (auto it = this->pred_##TYPE##_begin(), ie = this->pred_##TYPE##_end(); it != ie; it++) { \
            Self *target = *it;                                                                       \
            auto iter = target->succCons[index].find(this);                                           \
            assert(iter != target->succCons[index].end());                                            \
            target->succCons[index].erase(iter);                                                      \
        }                                                                                             \
        this->succCons[index].clear();                                                                \
        this->predCons[index].clear();                                                                \
    }

        CLEAR_CONSTRAINT(load)
        CLEAR_CONSTRAINT(store)
        CLEAR_CONSTRAINT(copy)
        CLEAR_CONSTRAINT(addr_of)
        CLEAR_CONSTRAINT(offset)

#undef CLEAR_CONSTRAINT
    }

    [[nodiscard]] inline CGNodeKind getType() const { return type; }

    [[nodiscard]]
    inline bool hasSuperNode() const {
        return superNode != nullptr;
    }

    inline void setIndirectCallNode(CallGraphNode<ctx> *callNode) {
        // assert(callNode->isIndirectCall() && this->indirectNode == nullptr);
        this->indirectNodes.insert(callNode);
        // this->indirectNode = callNode;
    }

    inline const IndirectNodeSet &getIndirectNodes() const {
        return indirectNodes;
    }

    inline auto indirect_begin() const -> decltype(this->indirectNodes.begin()) {
        return this->indirectNodes.begin();
    }

    inline auto indirect_end() const -> decltype(this->indirectNodes.end()) {
        return this->indirectNodes.end();
    }

    [[nodiscard]]
    inline bool isFunctionPtr() {
        return !this->indirectNodes.empty();
    }

    [[nodiscard]]
    inline const GraphTy *getGraph() {
        return this->graph;
    }

    [[nodiscard]]
    inline NodeID getNodeID() const { return id; }

    [[nodiscard]]
    virtual std::string toString() const = 0;
    virtual ~CGNodeBase() = default;

    using cg_iterator = typename SetTy::iterator;

#define __CONS_ITER__(DIRECTION, KIND, TYPE)                                                        \
    [[nodiscard]] inline cg_iterator DIRECTION##_##KIND##_##TYPE() {                                \
        constexpr auto index = static_cast<std::underlying_type<Constraints>::type>(Constraints::KIND); \
        static_assert(index < 5, "");                                                                   \
        return DIRECTION##Cons[index].TYPE();                                                       \
    }

#define __BI_CONS_ITER__(KIND, TYPE) \
    __CONS_ITER__(succ, KIND, TYPE)  \
    __CONS_ITER__(pred, KIND, TYPE)

#define DEFINE_CONS_ITER(KIND)    \
    __BI_CONS_ITER__(KIND, begin) \
    __BI_CONS_ITER__(KIND, end)

    // succ_load_begin, succ_load_end, pred_load_begin, pred_load_end
    DEFINE_CONS_ITER(load)
    DEFINE_CONS_ITER(store)
    DEFINE_CONS_ITER(copy)
    DEFINE_CONS_ITER(addr_of)
    DEFINE_CONS_ITER(offset)

#undef DEFINE_CONS_ITER
#undef __BI_CONS_ITER__
#undef __CONS_ITER__

    // TODO: use LLVM built-in concat interator, they have better implementation
    using iterator = ConcatIterator<typename SetTy::iterator, 5>;
    using const_iterator = ConcatIterator<typename SetTy::const_iterator, 5>;

    using edge_iterator = ConcatIteratorWithTag<typename SetTy::iterator, 5, Constraints>;
    using const_edge_iterator = ConcatIteratorWithTag<typename SetTy::const_iterator, 5, Constraints>;

#define INIT_ITERATOR(CONTAINER, BEGIN, END)     \
    (                                             \
        CONTAINER[4].BEGIN(), CONTAINER[4].END(), \
        CONTAINER[3].BEGIN(), CONTAINER[3].END(), \
        CONTAINER[2].BEGIN(), CONTAINER[2].END(), \
        CONTAINER[1].BEGIN(), CONTAINER[1].END(),  \
        CONTAINER[0].BEGIN(), CONTAINER[0].END()  \
    )

    inline iterator succ_begin() { return iterator INIT_ITERATOR(succCons, begin, end); }
    inline iterator succ_end() { return iterator INIT_ITERATOR(succCons, end, end); }
    inline const_iterator succ_begin() const { return const_iterator INIT_ITERATOR(succCons, begin, end); }
    inline const_iterator succ_end() const { return const_iterator INIT_ITERATOR(succCons, end, end); }

    inline iterator pred_begin() { return iterator INIT_ITERATOR(predCons, begin, end); }
    inline iterator pred_end() { return iterator INIT_ITERATOR(predCons, end, end); }
    inline const_iterator pred_begin() const { return const_iterator INIT_ITERATOR(predCons, begin, end); }
    inline const_iterator pred_end() const { return const_iterator INIT_ITERATOR(predCons, end, end); }

    inline edge_iterator succ_edge_begin() {
        return edge_iterator INIT_ITERATOR(succCons, begin, end);
    }

    inline edge_iterator succ_edge_end() {
        return edge_iterator INIT_ITERATOR(succCons, end, end);
    }

    inline const_edge_iterator succ_edge_begin() const {
        return const_edge_iterator INIT_ITERATOR(succCons, begin, end);
    }

    inline const_edge_iterator succ_edge_end() const {
        return const_edge_iterator INIT_ITERATOR(succCons, end, end);
    }

    inline edge_iterator edge_begin() {
        return succ_edge_begin();
    }

    inline edge_iterator edge_end() {
        return succ_edge_end();
    }

    inline const_edge_iterator edge_begin() const {
        return succ_edge_begin();
    }

    inline const_edge_iterator edge_end() const {
        return succ_edge_end();
    }

    inline edge_iterator pred_edge_begin() {
        return edge_iterator INIT_ITERATOR(predCons, begin, end);
    }

    inline edge_iterator pred_edge_end() {
        return edge_iterator INIT_ITERATOR(predCons, end, end);
    }

    inline const_edge_iterator pred_edge_begin() const {
        return const_edge_iterator INIT_ITERATOR(predCons, begin, end);
    }

    inline const_edge_iterator pred_edge_end() const {
        return const_edge_iterator INIT_ITERATOR(predCons, end, end);
    }

#undef INIT_ITERATOR

    friend class GraphBase<Self, Constraints>;
    friend class ConstraintGraph<ctx>;
};

}  // namespace aser

#endif