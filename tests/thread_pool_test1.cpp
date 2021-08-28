#include "gtest/gtest.h"
#include "quick_sort.h"

TEST(QuickSort, QuickSortFunction1) {
    std::list<int> arrays = {
      9, 8, 6, 123, 352, 5235,
      423, 10, 356, 235, 5146, 1243,
      1234, 124, 235, 643, 6367, 7457,
    };
    arrays = thread::parallel_quick_sort(arrays);
    std::cout << "arrays:[";
    for (auto &i : arrays) {
      std::cout << i << ",";
    }
    std::cout << "]\n";
}