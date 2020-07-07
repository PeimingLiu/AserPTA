//
// Created by peiming on 9/16/19.
//
#ifndef ASER_PTA_WAVEPROPAGATION_H
#define ASER_PTA_WAVEPROPAGATION_H

#include "SolverBase.h"
#include "aser/PointerAnalysis/Graph/ConstraintGraph/SCCIterator.h"

namespace aser {

template <typename LangModel>
class WavePropagation : public SolverBase<LangModel, WavePropagation<LangModel>> {
public:
    using super = SolverBase<LangModel, WavePropagation<LangModel>>;
    using PtsTy = typename super::PtsTy;
    using PT = PTSTrait<PtsTy>;
    using ConsGraphTy = ConstraintGraph<typename super::ctx>;
    using CGNodeTy = CGNodeBase<typename super::ctx>;
    using GT = llvm::GraphTraits<ConsGraphTy>;
    using ctx = typename super::ctx;
    using LMT = typename super::LMT;

    using Edge = std::tuple<const CGNodeTy *, const CGNodeTy *, Constraints>;

    std::vector<typename PT::PtsTy> cachedPTS;
    std::map<Edge, typename PT::PtsTy> edgeCachedPTS;

protected:
    void runSolver(LangModel &langModel) {
        bool changed;
        ConsGraphTy &consGraph = *(super::getConsGraph());
        cachedPTS.clear();

        do {
            changed = false;

            // enlarge the cachedPTS if necessary, after processing offset constraints, new node can be added
            while (cachedPTS.size() != consGraph.getNodeNum()) {
                cachedPTS.emplace_back();
            }

            std::stack<std::vector<CGNodeTy *>> copySCCStack;
            // first do SCC detection and topo-sort
            auto copy_it = scc_begin<ctx, Constraints::copy, false>(consGraph);
            auto copy_ie = scc_end<ctx, Constraints::copy, false>(consGraph);

            for (; copy_it != copy_ie; ++copy_it) {
                copySCCStack.push(*copy_it);
            }

            while (!copySCCStack.empty()) {
                const std::vector<CGNodeTy *> &scc = copySCCStack.top();
                if (scc.size() > 1) {
                    auto superNode = super::processCopySCC(scc);
                    cachedPTS[superNode->getNodeID()] = PT::getPointsTo(superNode->getNodeID());
                } else {
                    CGNodeTy *curNode = scc.front();
                    typename PT::PtsTy diffPTS;
                    diffPTS.intersectWithComplement(PT::getPointsTo(curNode->getNodeID()),
                                                    cachedPTS[curNode->getNodeID()]);

                    // only when the diffed pts is not empty
                    if (!diffPTS.empty()) {
                        cachedPTS[curNode->getNodeID()] |= diffPTS;
                        for (auto cit = curNode->succ_copy_begin(), cie = curNode->succ_copy_end(); cit != cie; cit++) {
                            super::processCopy(curNode, *cit);
                        }
                    }
                }
                copySCCStack.pop();
            }

            // then handle STORE/LOAD edges.
            size_t nodeNum = consGraph.getNodeNum();
            for (NodeID id = 0; id < nodeNum; id++) {
                CGNodeTy *curNode = consGraph.getNode(id);

                for (auto it = curNode->succ_load_begin(), ie = curNode->succ_load_end(); it != ie; it++) {
                    auto &cachedPts = edgeCachedPTS[std::make_tuple(curNode, *it, Constraints::load)];
                    typename PT::PtsTy diffPTS;
                    diffPTS.intersectWithComplement(PT::getPointsTo(curNode->getNodeID()),
                                                    cachedPts);
                    cachedPts |= diffPTS;
                    changed |= super::processLoad(curNode, *it, [&](CGNodeTy *src, CGNodeTy *dst) {
                      super::processCopy(src, dst);
                    }, &diffPTS);

                }

                for (auto it = curNode->pred_store_begin(), ie = curNode->pred_store_end(); it != ie; it++) {
                    auto &cachedPts = edgeCachedPTS[std::make_tuple(curNode, *it, Constraints::store)];
                    typename PT::PtsTy diffPTS;
                    diffPTS.intersectWithComplement(PT::getPointsTo(curNode->getNodeID()),
                                                    cachedPts);
                    cachedPts |= diffPTS;

                    changed |= super::processStore(*it, curNode, [&](CGNodeTy *src, CGNodeTy *dst) {
                      super::processCopy(src, dst);
                    }, &diffPTS);
                }

                for (auto it = curNode->succ_offset_begin(), ie = curNode->succ_offset_end(); it != ie; it++) {
                    // offset already cached the diff points to set
                    changed |= super::processOffset(curNode, *it, [&](CGNodeTy *src, CGNodeTy *dst) {
                      auto addrNode = llvm::cast<typename super::ObjNodeTy>(src)->getAddrTakenNode();
                      super::processCopy(addrNode, dst);
                    });
                }
            }
        } while (changed);
    }

    friend super;
};

}  // namespace aser

#endif