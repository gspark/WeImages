#ifndef CNATURALSORTING_H
#define CNATURALSORTING_H

#include "sortingoptions.h"
#include "naturalsorterqcollator.h"

enum NaturalSortingAlgorithm {
    nsaQtForum,
    nsaQCollator
};

class QString;

class NaturalSorting {
public:
    NaturalSorting(NaturalSortingAlgorithm algorithm, SortingOptions options);

    void setSortingOptions(SortingOptions options);

    void setSortingAlgorithm(NaturalSortingAlgorithm algorithm);

    bool lessThan(const QString &l, const QString &r) const;

    bool equal(const QString &l, const QString &r) const;

    int compare(const QString &l, const QString &r) const;

private:
    SortingOptions _options;
    NaturalSortingAlgorithm _algorithm;

    NaturalSorterQCollator _collatorSorter;
};

#endif // CNATURALSORTING_H
