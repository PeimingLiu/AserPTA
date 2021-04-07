//
// Created by peiming on 3/30/20.
//

#ifndef ASER_PTA_STATISTICS_H
#define ASER_PTA_STATISTICS_H

#undef LLVM_FORCE_ENABLE_STATS
#define LLVM_FORCE_ENABLE_STATS 1

#define LLVM_ENABLE_STATS 1

#include <llvm/ADT/Statistic.h>

#define LOCAL_STATISTIC(VARNAME, DESC)                                               \
  llvm::Statistic VARNAME {DEBUG_TYPE, #VARNAME, DESC, {0}, {false}}

#endif  // ASER_PTA_STATISTICS_H
