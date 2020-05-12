//
// Created by peiming on 11/2/19.
//
#ifndef ASER_PTA_CGOBJNODE_H
#define ASER_PTA_CGOBJNODE_H

#include "CGNodeBase.h"

namespace aser {

template <typename MemModel>
struct MemModelTrait;
template <typename ctx>
class ConstraintGraph;

// nodes represent objects
template <typename MemModel>
class CGObjNode : public CGNodeBase<typename MemModelTrait<MemModel>::CtxTy> {
private:
    using Self = CGObjNode<MemModel>;
    // memory model trait
    using MMT = MemModelTrait<MemModel>;
    // context
    using ctx = typename MMT::CtxTy;
    using super = CGNodeBase<ctx>;

    using T = typename MMT::ObjectTy;
    const T *obj;

    CGObjNode(const T *obj, NodeID id) : super(id, CGNodeKind::ObjNode), obj(obj){};

public:
    static inline bool classof(const super *node) {
        return node->getType() == CGNodeKind::ObjNode;
    }

    inline CGNodeBase<ctx> *getAddrTakenNode() {
        CGNodeBase<ctx> *addrTakeNode = this->graph->getNode(this->getNodeID() + 1);
        assert(addrTakeNode->getType() == CGNodeKind::PtrNode);

        return addrTakeNode;
    }

    inline const T *getObject() const {
        return obj;
    }

    friend class ConstraintGraph<ctx>;

    [[nodiscard]] std::string toString() const {
        std::string str;
        llvm::raw_string_ostream os(str);
        if (this->isSuperNode()) {
            os << "SuperNode: \n";
            llvm::dump(this->childNodes, os);
        } else {
            os << super::getNodeID() << "\n";
            os << obj->toString() << "\n";
        }
        return os.str();
    }

    friend class GraphBase<super, Constraints>;
    friend class ConstraintGraph<ctx>;
};

}  // namespace aser

#endif