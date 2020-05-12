//
// Created by peiming on 8/14/19.
//
#ifndef ASER_PTA_UTIL_H
#define ASER_PTA_UTIL_H

#include <llvm/ADT/iterator.h>
#include <llvm/IR/CallSite.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Operator.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/IR/Type.h>

#include <queue>

namespace aser {

inline llvm::raw_ostream &dbg_os() {
#ifndef NDEBUG
    return llvm::dbgs();
#else
    return llvm::nulls();
#endif
}

template <typename BOOL>
bool AndBools(BOOL b) {
    return b;
}

template <typename BOOL, typename ...BOOLS>
bool AndBools(BOOL cur, BOOLS ...remain) {
    if (cur) {
        return AndBools(remain...);
    }
    return false;
}


template <typename T, size_t ...N>
bool isTupleEquelOnIndex(const T& T1, const T& T2, llvm::index_sequence<N...> sequence) {
    return AndBools((std::get<N>(T1)==std::get<N>(T2))...);
}

template <typename ...Args>
bool isTupleEqual(const std::tuple<Args...> &tuple1, const std::tuple<Args...> &tuple2) {
    static_assert(sizeof ... (Args) > 0);

    return isTupleEquelOnIndex(tuple1, tuple2, llvm::index_sequence_for<Args...>{});
}

// instead of compare pointer value, compare they content of them
template <typename PtrTy, typename Comparator = std::less<PtrTy>>
class PtrContentComparator : public std::binary_function<PtrTy *, PtrTy *, bool> {
    using comp = Comparator;
    comp _M_comp;

public:
    bool operator()(const PtrTy *__x, const PtrTy *__y) { return _M_comp(*__x, *__y); }
};

template <typename PtrTy, typename Comparator = std::less<PtrTy>>
class UniquePtrContentComparator : public std::binary_function<std::unique_ptr<PtrTy>, std::unique_ptr<PtrTy>, bool> {
    using comp = Comparator;
    comp _M_comp;

public:
    bool operator()(const std::unique_ptr<PtrTy> &__x, const std::unique_ptr<PtrTy> &__y) const {
        return _M_comp(*(__x.get()), *(__y.get()));
    }
};

inline llvm::Type *getBoundedArrayTy(llvm::Type *elemType, size_t num) {
    if (llvm::ArrayType::isValidElementType(elemType)) {
        return llvm::ArrayType::get(elemType, num);
    }

    return nullptr;
}


inline llvm::Type *getUnboundedArrayTy(llvm::Type *elemType) {
    return getBoundedArrayTy(elemType, std::numeric_limits<uint64_t>::max());
}


// TODO: make it a ring buffer
// Vector but with fixed size
// pop out the first element when exceeding the capacity.
// used for k-limiting context
template <typename ElemType>
class FixSizedVector {
private:
    std::vector<ElemType> vec;
    const uint capacity;

    const std::vector<ElemType> &getVec() const { return vec; }

public:
    using iterator = typename std::vector<ElemType>::iterator;
    using const_iterator = typename std::vector<ElemType>::const_iterator;

    inline explicit FixSizedVector(int size) : capacity(size) { vec.reserve(size); }

    inline FixSizedVector(const FixSizedVector &v) : capacity(v.getCapacity()), vec(v.getVec()) {}
    inline FixSizedVector(FixSizedVector &&v) noexcept : capacity(v.getCapacity()), vec(std::move(v.getVec())) {}

    inline void push_bask(const ElemType &elem) {
        // push a new element in the back, remove the beginning one if out of
        // storage
        // TODO: maybe a ring buffer is faster
        if (vec.size() == capacity) {
            vec.erase(vec.begin());
        }
        vec.push_back(elem);
        assert(vec.size() <= capacity);
    }

    [[nodiscard]] inline uint getCapacity() const { return capacity; }

    [[nodiscard]] inline uint size() const { return vec.size(); }

    inline iterator begin() { return vec.begin(); }
    inline iterator end() { return vec.end(); }
    inline const_iterator begin() const { return vec.begin(); }
    inline const_iterator end() const { return vec.end(); }
    inline const_iterator cbegin() const { return vec.cbegin(); }
    inline const_iterator cend() const { return vec.cend(); }

    inline const ElemType &front() const { return vec.front(); }
    inline const ElemType &back() const { return vec.back(); }
};

void prettyFunctionPrinter(const llvm::Function *func, llvm::raw_ostream &os);

// whether the indirect call site compatible with the target function
bool isCompatibleCall(const llvm::Instruction *indirectCall, const llvm::Function *target);

}  // namespace aser

#endif