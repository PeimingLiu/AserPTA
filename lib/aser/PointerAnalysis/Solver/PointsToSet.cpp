//
// Created by peiming on 10/23/19.
//

#include "aser/PointerAnalysis/Solver/PointsTo/BitVectorPTS.h"
#include "aser/PointerAnalysis/Solver/PointsTo/PointedByPts.h"

namespace aser {

std::vector<BitVectorPTS::PtsTy> BitVectorPTS::ptsVec;
std::vector<PointedByPts::PtsTy> PointedByPts::pointsTo;
std::vector<PointedByPts::PtsTy> PointedByPts::pointedBy;

}  // namespace aser