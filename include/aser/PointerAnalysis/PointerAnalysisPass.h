//
// Created by peiming on 3/24/20.
//

#ifndef ASER_PTA_POINTERANALYSISPASS_H
#define ASER_PTA_POINTERANALYSISPASS_H

#include <bits/unique_ptr.h>
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Pass.h>

template <typename Solver>
class PointerAnalysisPass : public llvm::ImmutablePass {
private:
    std::unique_ptr<Solver> solver;  // owner of the solver

public:
    static char ID;
    PointerAnalysisPass() : solver(nullptr), llvm::ImmutablePass(ID) {}

    void analyze(llvm::Module *M, llvm::StringRef entry = "main") {
        if (solver.get() != nullptr) {
            if (solver->getLLVMModule() == M && entry.equals(solver->getEntryName())) {
                return;
            }
        }
        // release previous context
        solver.reset(new Solver());

        llvm::outs() << "PTA start to run\n";
        auto start = std::chrono::steady_clock::now();
        solver->analyze(M, entry);

        auto end = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed_seconds = end-start;

        llvm::outs() << "PTA finished, running time : "
                     << std::chrono::duration_cast<std::chrono::milliseconds>(elapsed_seconds).count() << " ms\n";
    }

    Solver *getPTA() const {
        assert(solver.get() != nullptr && "call analyze() before getting the pta instance");
        return solver.get();
    }

    void release() {
        // release the memory hold by the correct solver
        solver.reset(nullptr);
    }
};

template <typename Solver>
char PointerAnalysisPass<Solver>::ID = 0;

#endif  // ASER_PTA_POINTERANALYSISPASS_H
