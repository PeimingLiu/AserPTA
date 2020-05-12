//
// Created by peiming on 12/19/19.
//

#ifndef ASER_PTA_FSOBJECT_H
#define ASER_PTA_FSOBJECT_H

#include "aser/PointerAnalysis/Models/MemoryModel/AllocSite.h"

namespace aser {
// forward declaration
using NodeID = uint64_t;
template <typename MemModel> class CGObjNode;
template <typename ctx> class FSMemModel;
template <typename ctx> class MemBlock;

template <typename ctx>
class FSObject {
private:
    using CT = CtxTrait<ctx>;
    using ObjNode = CGObjNode<FSMemModel<ctx>>;

    MemBlock<ctx> *const memBlock;
    size_t pOffset; // the physical offset (the allocation offset)
    size_t lOffset; // layout offset (how to index the memblock)

    ObjNode* objNode = nullptr;
    // this can only be called internally
    inline void setObjNode(ObjNode* node) {
        assert(objNode == nullptr);
        objNode = node;
    }

    [[nodiscard]] inline ObjNode* getObjNodeOrNull() const {
        return objNode;
    }

    inline const FSObject<ctx> * getObjIndexedByVar(size_t stepSize) const {
        // when we encounter an object that indexed by variable, we need to ensure that we
        // are indexing an array
        // although C++/C allows using variable to index structure, but it is rare in practices, and
        // we ignore it at the current stage.
        if (memBlock->validateStepSize(this->pOffset, stepSize)) {
            return this;
        }
        return nullptr;
    }

//    result shows it is not helpful

//    llvm::DenseMap<const llvm::GetElementPtrInst *, ObjNode *> cachedResult;
//    inline std::pair<ObjNode *, bool> getCachedResult(const llvm::GetElementPtrInst *key) const {
//        auto it = cachedResult.find(key);
//        if (it != cachedResult.end()) {
//            return std::make_pair(it->second, true);
//        }
//
//        return std::make_pair(nullptr, false);
//    }
//
//    inline void cacheIndexResult(const llvm::GetElementPtrInst *key, ObjNode *value) const {
//        const_cast<FSObject<ctx> *>(this)->cachedResult.insert(std::make_pair(key, value));
//    }

public:
    FSObject(MemBlock<ctx> *memBlock, size_t pOffset, size_t lOffset)
        : memBlock(memBlock), pOffset(pOffset), lOffset(lOffset) {};

    // can not be moved/copied
    FSObject(const FSObject<ctx>&) = delete;
    FSObject(FSObject<ctx>&&) = delete;
    FSObject<ctx>& operator=(const FSObject<ctx>&) = delete;
    FSObject<ctx>& operator=(FSObject<ctx>&&) = delete;

    [[nodiscard]] inline const AllocSite<ctx>& getAllocSite() const {
        return this->memBlock->getAllocSite();
    }

    [[nodiscard]] inline const ctx* getContext() const {
        return this->getAllocSite().getContext();
    }

    [[nodiscard]] inline const llvm::Value* getValue() const {
        return this->getAllocSite().getValue();
    }

    [[nodiscard]] inline ObjNode* getObjNode() const {
        assert(objNode != nullptr);
        return objNode;
    }

    [[nodiscard]] inline AllocType getAllocType() const {
        return this->getAllocSite().getAllocType();
    }

    [[nodiscard]] inline bool isFunction() const {
        return this->getAllocType() == AllocType::Functions;
    }

    [[nodiscard]] inline const llvm::Type* getType() const {
        return this->getAllocSite().getValue()->getType();
    }

    [[nodiscard]] inline bool isGlobalObj() const {
        return this->getAllocType() == AllocType::Globals;
    }

    [[nodiscard]] inline bool isStackObj() const {
        return this->getAllocType() == AllocType::Stack;
    }

    [[nodiscard]] inline bool isHeapObj() const {
        return this->getAllocType() == AllocType::Heap;
    }

    [[nodiscard]] inline bool isFIObject() const {
        return this->memBlock->isFIBlock();
    }

    [[nodiscard]] inline std::string toString(bool detailed = true) const {
        if (detailed) {
            std::string ctxStr = CT::toString(getContext(), detailed);
            llvm::raw_string_ostream os(ctxStr);
            if (getAllocType() == AllocType::Anonymous) {
                os << "\n Anonymous";
            } else if (llvm::isa<llvm::Function>(getValue())) {
                os << getValue()->getName();
            } else if (getValue()->hasName()) {
                os << "\n" << *getValue();
            }
            os << "\nOffset:" << pOffset;
            return os.str();
        } else {
            if (getValue()->hasName()) {
                return getValue()->getName();
            }
            return "";
        }
    }

    friend FSMemModel<ctx>;
    friend CGObjNode<FSMemModel<ctx>>;
};

}

#endif  // ASER_PTA_FSOBJECT_H
