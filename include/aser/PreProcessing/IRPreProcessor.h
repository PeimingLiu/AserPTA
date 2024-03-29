//
// Created by peiming on 2/26/20.
//

#ifndef ASER_PTA_IRPREPROCESSOR_H
#define ASER_PTA_IRPREPROCESSOR_H

#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>


namespace aser {

// maybe just a PreprocessingModule function is enough, we make it as a class in case in the future
// we might want to configure it.
class IRPreProcessor {
public:
    IRPreProcessor() = default;
    void runOnModule(llvm::Module &M);
};

}

#endif  // ASER_PTA_IRPREPROCESSOR_H
