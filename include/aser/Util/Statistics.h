//
// Created by peiming on 3/30/20.
//

#ifndef ASER_PTA_STATISTICS_H
#define ASER_PTA_STATISTICS_H

#include <llvm/ADT/Statistic.h>

#ifndef NO_STATS
#define LOCAL_STATISTIC(VARNAME, DESC) \
  llvm::Statistic VARNAME {DEBUG_TYPE, #VARNAME, DESC, {0}, {false}}
#else
#define LOCAL_STATISTIC(VARNAME, DESC) \
  int VARNAME = 0;
#endif

#endif  // ASER_PTA_STATISTICS_H
