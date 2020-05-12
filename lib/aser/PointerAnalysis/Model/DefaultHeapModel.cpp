//
// Created by peiming on 1/16/20.
//
#include "aser/PointerAnalysis/Models/DefaultHeapModel.h"
#include "aser/Util/Util.h"

#include <aser/PointerAnalysis/Program/CallSite.h>
#include <llvm/IR/Instructions.h>

using namespace llvm;
using namespace aser;

static Type *getNextBitCastDestType(const Instruction *allocSite) {
    // a call instruction
    const Instruction *nextInst = nullptr;
    if (auto call = dyn_cast<CallInst>(allocSite)) {
        nextInst = call->getNextNode();
    } else if (auto invoke = dyn_cast<InvokeInst>(allocSite)) {
        // skip the exception handler code
        nextInst = invoke->getNormalDest()->getFirstNonPHIOrDbgOrLifetime();
    }

    if (nextInst && isa<BitCastInst>(nextInst)) {
        Type *destTy = cast<BitCastInst>(nextInst)->getDestTy()->getPointerElementType();
        if (destTy->isSized()) {
            // only when the dest type is sized
            return destTy;
        }
    }

    return nullptr;
}

// the signature of calloc is void *calloc(size_t elementNum, size_t elementSize);
Type *DefaultHeapModel::inferCallocType(const Function *fun, const Instruction *allocSite,
                                        int numArgNo, int sizeArgNo) {
    if (auto elemType = getNextBitCastDestType(allocSite)) {
        assert(elemType->isSized());

        aser::CallSite CS(allocSite);
        const DataLayout &DL = fun->getParent()->getDataLayout();
        const size_t elemSize = DL.getTypeAllocSize(elemType);
        const Value *elementNum = CS.getArgOperand(numArgNo);
        const Value *elementSize = CS.getArgOperand(sizeArgNo);

        if (auto size = dyn_cast<ConstantInt>(elementSize)) {
            if (elemSize == size->getSExtValue()) {
                // GREAT, we are sure that the element type is the bitcast type
                if (auto elemNum = dyn_cast<ConstantInt>(elementNum)) {
                    return getBoundedArrayTy(elemType, elemNum->getSExtValue());
                } else {
                    // the element number can not be determined
                    return getUnboundedArrayTy(elemType);
                }
            }
        }
    }
    return nullptr;
}

// the signature of malloc is void *malloc(size_t elementSize);
Type *DefaultHeapModel::inferMallocType(const Function *fun, const Instruction *allocSite,
                                        int sizeArgNo) {

    if (auto elemType = getNextBitCastDestType(allocSite)) {
        assert(elemType->isSized());

        // if the sizeArgNo is not specified, treat it as unbounded array
        if (sizeArgNo < 0) {
            return getUnboundedArrayTy(elemType);
        }

        aser::CallSite CS(allocSite);
        const DataLayout &DL = fun->getParent()->getDataLayout();
        const size_t elemSize = DL.getTypeAllocSize(elemType);
        const Value *totalSize = CS.getArgOperand(sizeArgNo);

        // the allocated object size is known statically
        if (auto constSize = dyn_cast<ConstantInt>(totalSize)) {
            size_t memSize = constSize->getValue().getSExtValue();
            if (memSize == elemSize) {
                // GREAT!
                return elemType;
            } else if (memSize % elemSize == 0) {
                return getBoundedArrayTy(elemType, memSize / elemSize);
            }
            return nullptr;
        } else {
            // the size of allocated heap memory is unknown.
            // treat is an array with infinite elements and the ty
            if (DL.getTypeAllocSize(elemType) == 1) {
                // a int8_t[] is equal to a field-insensitive object.
                return nullptr;
            } else {
                return getUnboundedArrayTy(elemType);
            }
        }
    }

    // we can not resolve the type
    return nullptr;
}
