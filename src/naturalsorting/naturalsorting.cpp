#include "naturalsorting.h"
#include "naturalsorting_qt.h"

NaturalSorting::NaturalSorting(NaturalSortingAlgorithm algorithm, SortingOptions options) : _options(options),
                                                                                            _algorithm(algorithm) {
}

void NaturalSorting::setSortingOptions(SortingOptions options) {
    _options = options;
}

void NaturalSorting::setSortingAlgorithm(NaturalSortingAlgorithm algorithm) {
    _algorithm = algorithm;
}

bool NaturalSorting::lessThan(const QString &l, const QString &r) const {
    switch (_algorithm) {
        case nsaQtForum:
            return naturalSortComparisonQt(l, r, _options);
        case nsaQCollator:
            return _collatorSorter.compare(l, r, _options);
        default:
            return false;
    }
}

bool NaturalSorting::equal(const QString &l, const QString &r) const {
    return compare(l, r) == 0;
}

int NaturalSorting::compare(const QString &l, const QString &r) const {
    if (lessThan(l, r))
        return -1;
    else if (lessThan(r, l))
        return 1;
    else
        return 0;
}
