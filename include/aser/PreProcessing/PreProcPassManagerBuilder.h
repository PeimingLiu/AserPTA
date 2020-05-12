//
// Created by peiming on 2/26/20.
//
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>

#ifndef ASER_PTA_PREPROCPASSMANAGERBUILDER_H
#define ASER_PTA_PREPROCPASSMANAGERBUILDER_H

namespace aser {

enum class UseCFLAA { None, Steensgaard, Andersen, Both };

// inspired by llvm::PassManagerBuilder
class PreProcPassManagerBuilder {
private:
    UseCFLAA useCFL;
    bool runInstCombine;
    bool enableLoopUnswitch;
    bool enableSimpleLoopUnswitch;

    void addFunctionSimplificationPasses(llvm::legacy::PassManagerBase &MPM);

public:
    PreProcPassManagerBuilder();

    void populateModulePassManager(llvm::legacy::PassManagerBase &MPM);
    void populateFunctionPassManager(llvm::legacy::FunctionPassManager &FPM);
};

}

#endif  // ASER_PTA_PREPROCPASSMANAGERBUILDER_H
