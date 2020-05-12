//
// Created by peiming on 1/3/20.
//

#include "aser/PointerAnalysis/Models/MemoryModel/FieldSensitive/Layout/ArrayLayout.h"
#include "aser/PointerAnalysis/Models/MemoryModel/FieldSensitive/Layout/MemLayout.h"

namespace aser {

static size_t indexBetweenArrays(const std::map<size_t, ArrayLayout *> &arrays, size_t &pOffset) {
    size_t lOffset = 0;
    size_t curOffset = 0;
    for (auto arrayPair : arrays) {
        // std::map is ordered by key from small value to large value
        size_t arrayOffset = arrayPair.first;
        const ArrayLayout *arrayLayout = arrayPair.second;

        // 1st, may be between two arrays
        if (pOffset <= arrayOffset) {
            lOffset += pOffset - curOffset;
            return lOffset;
        }

        assert(arrayOffset >= curOffset);
        lOffset += arrayOffset - curOffset;
        // 2nd, may be larger than current array
        size_t arrayEnd = arrayOffset + arrayLayout->getArraySize();
        if (pOffset >= arrayOffset + arrayLayout->getArraySize()) {
            // the physical offset bypass the current array
            // accumulate offsets and skip to next array
            lOffset += arrayLayout->getLayoutSize();
            curOffset = arrayEnd;
            continue;
        }

        // 3rd, maybe in the middle of an array. ( start offset < pOffset < start offset + array size)
        size_t relativeOffset = pOffset - arrayOffset;
        size_t result = arrayLayout->indexPhysicalOffset(relativeOffset) + lOffset;
        // relative offset might be shrink when index array
        // i.e., a[0], a[1] is the same in our pointer analysis.
        pOffset = arrayOffset + relativeOffset;
        return result;
    }

    // 4th, after the last array.
    lOffset += pOffset - curOffset;
    return lOffset;
}

size_t MemLayout::indexPhysicalOffset(size_t &pOffset) const {
    if (!this->hasArray()) {
        // fast path
        // if the memory block does not have array ==> physical offset == layout offset
        return pOffset;
    } else if (pOffset >= maxPOffset) {
        // invalid offset (exceed the max physical offset)
        return std::numeric_limits<size_t>::max();
    } else {
        // :-(, now we have to translate the physical offset
        return indexBetweenArrays(this->subArrays, pOffset);
    }
}

// merge a sub-layout into current memory layout
void MemLayout::mergeMemoryLayout(const MemLayout *subLayout, size_t pOffset, size_t lOffset) {
    for (auto elem : subLayout->elementLayout) {
        elementLayout.set(elem + lOffset);
    }
    for (auto elem : subLayout->pointerLayout) {
        pointerLayout.set(elem + lOffset);
    }

    assert(elementLayout.contains(pointerLayout));

    // TODO: make sure there is no overlapping between arrays
    // merge the array layout
    if (mIsArray) {
        assert(subArrays.size() == 1 && subArrays.begin()->first == 0);
        subArrays.begin()->second->mergeSubArrays(subLayout->subArrays, 0);
    } else {
        for (auto subArray : subLayout->subArrays) {
            subArrays.insert(std::make_pair(subArray.first + pOffset, subArray.second));
        }
    }
}

size_t ArrayLayout::indexPhysicalOffset(size_t &pOffset) const {
    if (this->hasSubArrays()) {
        pOffset = pOffset % this->getElementSize();
        return indexBetweenArrays(this->subArrays, pOffset);
    } else {
        assert(pOffset <= this->getArraySize());
        // we do not distinguish elements in the arrays.
        pOffset = pOffset % this->getElementSize();
        return pOffset;
    }
}

}  // namespace aser