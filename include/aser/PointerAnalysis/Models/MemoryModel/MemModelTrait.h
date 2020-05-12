//
// Created by peiming on 10/22/19.
//
#ifndef ASER_PTA_MEMMODELTRAIT_H
#define ASER_PTA_MEMMODELTRAIT_H

namespace aser {

// this class handles static object modelling,
// e.g., field sensitive
template <typename MemModel>
struct MemModelTrait {
    // context type
    using CtxTy = typename MemModel::UnknownTypeError;

    using ObjectTy = typename MemModel::UnknownTypeError;

    // CGObjNode<MemModel>* allocateNullObj();

    // CGObjNode<MemModel>* allocateUniObj();
};



}  // namespace aser

#endif