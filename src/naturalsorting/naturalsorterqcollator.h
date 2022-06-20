#pragma once

#include "sortingoptions.h"
#include <QCollator>

class NaturalSorterQCollator {
public:
    NaturalSorterQCollator();

    bool compare(const QString &l, const QString &r, SortingOptions options) const;

private:
    QCollator _collator;
};
