//
// Created by peiming on 8/14/19.
//
#include "aser/Util/Util.h"

#include <aser/PointerAnalysis/Program/CallSite.h>

using namespace llvm;
using namespace aser;

void aser::prettyFunctionPrinter(const Function *func, raw_ostream &os) {
    os << *func->getReturnType() << " @" << func->getName() << "(";
    auto funcType = func->getFunctionType();
    for (unsigned I = 0, E = funcType->getNumParams(); I != E; ++I) {
        if (I) os << ", ";
        os << *funcType->getParamType(I);
    }
    if (funcType->isVarArg()) {
        if (funcType->getNumParams()) os << ", ";
        os << "...";  // Output varargs portion of signature!
    }
    os << ")";
}

// simple type check fails when cases like
// call void (...) %ptr()
bool aser::isCompatibleCall(const llvm::Instruction *indirectCall, const llvm::Function *target) {
    aser::CallSite CS(indirectCall);
    assert(CS.isIndirectCall());

    // fast path, the same type
    if (CS.getCalledValue()->getType() == target->getType()) {
        return true;
    }

    if (CS.getType() != target->getReturnType()) {
        return false;
    }

    if (CS.getNumArgOperands() != target->arg_size() && !target->isVarArg()) {
        // two non-vararg function should at have same number of parameters
        return false;
    }

    if (target->isVarArg() && target->arg_size() > CS.getNumArgOperands()) {
        // calling a varargs function, the callsite should offer at least the
        // same number of parameters required by var-args
        return false;
    }

    // LLVM IR is strongly typed, so ensure every actually argument is of the
    // same type as the formal arguments.
    auto fit = CS.arg_begin();
    for (const Argument &arg : target->args()) {
        const Value *param = *fit;
        if (param->getType() != arg.getType()) {
            return false;
        }
        fit++;
    }

    return true;
}
