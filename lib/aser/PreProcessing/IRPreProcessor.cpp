//
// Created by peiming on 2/26/20.
//
#include <llvm/ADT/Triple.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/CodeGen/CommandFlags.inc>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Analysis/TargetTransformInfo.h>

#include "aser/PreProcessing/IRPreProcessor.h"
#include "aser/PreProcessing/PreProcPassManagerBuilder.h"

using namespace llvm;
using namespace aser;

static TargetMachine* GetTargetMachine(Triple TheTriple, StringRef CPUStr,
                                       StringRef FeaturesStr,
                                       const TargetOptions &Options) {
    std::string Error;
    const Target *TheTarget = TargetRegistry::lookupTarget(MArch, TheTriple, Error);
    // Some modules don't specify a triple, and this is okay.
    if (!TheTarget) {
        return nullptr;
    }

    return TheTarget->createTargetMachine(TheTriple.getTriple(), CPUStr, FeaturesStr, Options, None);
}


void IRPreProcessor::runOnModule(llvm::Module &M) {
    // TODO: Do we really need to know the target machine information?
    Triple ModuleTriple(M.getTargetTriple());
    std::string CPUStr, FeaturesStr;
    TargetMachine *Machine = nullptr;
    const TargetOptions Options = InitTargetOptionsFromCodeGenFlags();

    if (ModuleTriple.getArch()) {
        CPUStr = getCPUStr();
        FeaturesStr = getFeaturesStr();
        Machine = GetTargetMachine(ModuleTriple, CPUStr, FeaturesStr, Options);
    } else if (ModuleTriple.getArchName() != "unknown" &&
               ModuleTriple.getArchName() != "") {
        // err: do not know target machine type
        return;
    }
    std::unique_ptr<TargetMachine> TM(Machine);

    // Override function attributes based on CPUStr, FeaturesStr, and command line
    // flags.
    // setFunctionAttributes(CPUStr, FeaturesStr, M);

    // Create a PassManager to hold and optimize the collection of passes we are
    // about to build.
    legacy::PassManager Passes;
    legacy::FunctionPassManager FPasses(&M);
    PreProcPassManagerBuilder builder;

    // Add an appropriate TargetLibraryInfo pass for the module's triple.
    // target-info and target transfromInfo
    TargetLibraryInfoImpl TLII(ModuleTriple);
    Passes.add(new TargetLibraryInfoWrapperPass(TLII));
    Passes.add(createTargetTransformInfoWrapperPass(TM ? TM->getTargetIRAnalysis() : TargetIRAnalysis()));

    FPasses.add(new TargetLibraryInfoWrapperPass(TLII));
    FPasses.add(createTargetTransformInfoWrapperPass(TM ? TM->getTargetIRAnalysis() : TargetIRAnalysis()));
    builder.populateFunctionPassManager(FPasses);
    builder.populateModulePassManager(Passes);

    FPasses.doInitialization();
    for (Function &F : M) {
        FPasses.run(F);
    }
    FPasses.doFinalization();

    Passes.run(M);
}