//
// Created by peiming on 1/10/20.
//

#include <llvm/Support/CommandLine.h>

using namespace llvm;

cl::opt<bool> ConfigPrintConstraintGraph("consgraph", cl::desc("Dump Constraint Graph to dot file"));
cl::opt<bool> ConfigPrintCallGraph("callgraph", cl::desc("Dump SHB graph to dot file"));
cl::opt<bool> ConfigDumpPointsToSet("dump-pts", cl::desc("Dump the Points-to Set of every pointer"));