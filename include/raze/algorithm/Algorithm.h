#pragma once

#include <raze/algorithm/batch/ForEach.h>
#include <raze/algorithm/batch/ForEachN.h>

#include <raze/algorithm/binary_search/BinarySearch.h>
#include <raze/algorithm/binary_search/EqualRange.h>
#include <raze/algorithm/binary_search/LowerBound.h>
#include <raze/algorithm/binary_search/UpperBound.h>

#include <raze/algorithm/copy/Copy.h>
#include <raze/algorithm/copy/CopyBackward.h>
#include <raze/algorithm/copy/CopyIf.h>
#include <raze/algorithm/copy/CopyN.h>
#include <raze/algorithm/copy/Move.h>
#include <raze/algorithm/copy/MoveBackward.h>

#include <raze/algorithm/extreme/Clamp.h>
#include <raze/algorithm/extreme/Max.h>
#include <raze/algorithm/extreme/MaxElement.h>
#include <raze/algorithm/extreme/Min.h>
#include <raze/algorithm/extreme/MinElement.h>
#include <raze/algorithm/extreme/Minmax.h>
#include <raze/algorithm/extreme/MinmaxElement.h>

#include <raze/algorithm/fill/Fill.h>
#include <raze/algorithm/fill/FillN.h>
#include <raze/algorithm/fill/Generate.h>
#include <raze/algorithm/fill/GenerateN.h>
#include <raze/algorithm/fill/GenerateRandom.h>

#include <raze/algorithm/find/AdjacentFind.h>
#include <raze/algorithm/find/AllOf.h>
#include <raze/algorithm/find/AnyOf.h>
#include <raze/algorithm/find/Contains.h>
#include <raze/algorithm/find/ContainsSubrange.h>
#include <raze/algorithm/find/Count.h>
#include <raze/algorithm/find/CountIf.h>
#include <raze/algorithm/find/CountIfNot.h>
#include <raze/algorithm/find/EndsWith.h>
#include <raze/algorithm/find/Equal.h>
#include <raze/algorithm/find/Find.h>
#include <raze/algorithm/find/FindEnd.h>
#include <raze/algorithm/find/FindFirstOf.h>
#include <raze/algorithm/find/FindIf.h>
#include <raze/algorithm/find/FindIfNot.h>
#include <raze/algorithm/find/FindLast.h>
#include <raze/algorithm/find/FindLastIf.h>
#include <raze/algorithm/find/FindLastIfNot.h>
#include <raze/algorithm/find/LexicographicalCompare.h>
#include <raze/algorithm/find/LexicographicalCompareThreeWay.h>
#include <raze/algorithm/find/Mismatch.h>
#include <raze/algorithm/find/NoneOf.h>
#include <raze/algorithm/find/Search.h>
#include <raze/algorithm/find/SearchN.h>
#include <raze/algorithm/find/StartsWith.h>

#include <raze/algorithm/fold/FoldLeft.h>
#include <raze/algorithm/fold/FoldLeftFirst.h>
#include <raze/algorithm/fold/FoldLeftFirstWithIter.h>
#include <raze/algorithm/fold/FoldLeftWithIter.h>
#include <raze/algorithm/fold/FoldRight.h>
#include <raze/algorithm/fold/FoldRightLast.h>

#include <raze/algorithm/heap/IsHeap.h>
#include <raze/algorithm/heap/IsHeapUntil.h>
#include <raze/algorithm/heap/MakeHeap.h>
#include <raze/algorithm/heap/PopHeap.h>
#include <raze/algorithm/heap/PushHeap.h>
#include <raze/algorithm/heap/SortHeap.h>

#include <raze/algorithm/numeric/Accumulate.h>
#include <raze/algorithm/numeric/AdjacentDifference.h>
#include <raze/algorithm/numeric/ExclusiveScan.h>
#include <raze/algorithm/numeric/InclusiveScan.h>
#include <raze/algorithm/numeric/InnerProduct.h>
#include <raze/algorithm/numeric/Iota.h>
#include <raze/algorithm/numeric/PartialSum.h>
#include <raze/algorithm/numeric/Reduce.h>
#include <raze/algorithm/numeric/TransformExclusiveScan.h>
#include <raze/algorithm/numeric/TransformInclusiveScan.h>
#include <raze/algorithm/numeric/TransformReduce.h>

#include <raze/algorithm/order/IsPermutation.h>
#include <raze/algorithm/order/IsSorted.h>
#include <raze/algorithm/order/IsSortedUntil.h>
#include <raze/algorithm/order/NextPermutation.h>
#include <raze/algorithm/order/PrevPermutation.h>
#include <raze/algorithm/order/Reverse.h>
#include <raze/algorithm/order/ReverseCopy.h>
#include <raze/algorithm/order/Rotate.h>
#include <raze/algorithm/order/RotateCopy.h>
#include <raze/algorithm/order/Sample.h>
#include <raze/algorithm/order/ShiftLeft.h>
#include <raze/algorithm/order/ShiftRight.h>
#include <raze/algorithm/order/Shuffle.h>

#include <raze/algorithm/partition/IsPartitioned.h>
#include <raze/algorithm/partition/Partition.h>
#include <raze/algorithm/partition/PartitionCopy.h>
#include <raze/algorithm/partition/PartitionPoint.h>
#include <raze/algorithm/partition/StablePartition.h>

#include <raze/algorithm/remove/Remove.h>
#include <raze/algorithm/remove/RemoveCopy.h>
#include <raze/algorithm/remove/RemoveCopyIf.h>
#include <raze/algorithm/remove/RemoveIf.h>
#include <raze/algorithm/remove/Unique.h>
#include <raze/algorithm/remove/UniqueCopy.h>

#include <raze/algorithm/replace/Replace.h>
#include <raze/algorithm/replace/ReplaceCopy.h>
#include <raze/algorithm/replace/ReplaceCopyIf.h>
#include <raze/algorithm/replace/ReplaceIf.h>

#include <raze/algorithm/set/Includes.h>
#include <raze/algorithm/set/InplaceMerge.h>
#include <raze/algorithm/set/Merge.h>
#include <raze/algorithm/set/SetDifference.h>
#include <raze/algorithm/set/SetIntersection.h>
#include <raze/algorithm/set/SetSymmetricDifference.h>
#include <raze/algorithm/set/SetUnion.h>

#include <raze/algorithm/sort/NthElement.h>
#include <raze/algorithm/sort/PartialSort.h>
#include <raze/algorithm/sort/PartialSortCopy.h>
#include <raze/algorithm/sort/Sort.h>
#include <raze/algorithm/sort/StableSort.h>

#include <raze/algorithm/swap/Swap.h>
#include <raze/algorithm/transform/Transform.h>

#include <raze/algorithm/uninitialized/ConstructAt.h>
#include <raze/algorithm/uninitialized/Destroy.h>
#include <raze/algorithm/uninitialized/DestroyAt.h>
#include <raze/algorithm/uninitialized/DestroyN.h>
#include <raze/algorithm/uninitialized/UninitializedCopy.h>
#include <raze/algorithm/uninitialized/UninitializedCopyN.h>
#include <raze/algorithm/uninitialized/UninitializedDefaultConstruct.h>
#include <raze/algorithm/uninitialized/UninitializedDefaultConstructN.h>
#include <raze/algorithm/uninitialized/UninitializedFill.h>
#include <raze/algorithm/uninitialized/UninitializedFillN.h>
#include <raze/algorithm/uninitialized/UninitializedMove.h>
#include <raze/algorithm/uninitialized/UninitializedMoveN.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstruct.h>
#include <raze/algorithm/uninitialized/UninitializedValueConstructN.h>
