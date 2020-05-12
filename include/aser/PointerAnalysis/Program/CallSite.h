//
// Created by peiming on 11/5/19.
//
#ifndef ASER_PTA_CALLSITE_H
#define ASER_PTA_CALLSITE_H

#include <llvm/IR/CallSite.h>

#include "aser/Util/Util.h"

namespace aser {

// wrapper around llvm::CallSite,
// but resolve constant expression evaluated to a function
class CallSite {
private:
    llvm::ImmutableCallSite CS;
    static const llvm::Function* resolveTargetFunction(const llvm::Value*);

public:
    explicit CallSite(const llvm::Instruction* I) : CS(I) {}

    [[nodiscard]] inline bool isCallOrInvoke() const { return CS.isCall() || CS.isInvoke(); }

    [[nodiscard]] inline bool isIndirectCall() const {
        if (CS.isIndirectCall()) {
            return true;
        }

        auto V = CS.getCalledValue();
        if (auto C = llvm::dyn_cast<llvm::Constant>(V)) {
            if (C->isNullValue()) {
                return true;
            }
        }

        return false;
    }

    [[nodiscard]] inline const llvm::Value* getCalledValue() const { return CS.getCalledValue(); }

    [[nodiscard]] inline const llvm::Function* getCalledFunction() const { return this->getTargetFunction(); }

    [[nodiscard]] inline const llvm::Function* getTargetFunction() const {
        if (this->isIndirectCall()) {
            return nullptr;
        }
        auto targetFunction = CS.getCalledFunction();
        if (targetFunction != nullptr) {
            return targetFunction;
        }

        return resolveTargetFunction(CS.getCalledValue());
    }

    [[nodiscard]]
    inline const llvm::Value* getReturnedArgOperand() const { return CS.getReturnedArgOperand(); }

    [[nodiscard]]
    inline const llvm::Instruction* getInstruction() const { return CS.getInstruction(); }

    [[nodiscard]]
    unsigned int getNumArgOperands() const { return CS.getNumArgOperands(); }

    const llvm::Value* getArgOperand(unsigned int i) const { return CS.getArgOperand(i); }

    inline auto args() const -> decltype(CS.args()) { return CS.args(); }

    [[nodiscard]]
    inline auto arg_begin() const -> decltype(CS.arg_begin()) { return CS.arg_begin(); }

    [[nodiscard]]
    inline auto arg_end() const -> decltype(CS.arg_end()) { return CS.arg_end(); }

    inline llvm::Type* getType() const { return CS.getType(); };
};

}  // namespace aser

#endif