#include "naturalsorterqcollator.h"

NaturalSorterQCollator::NaturalSorterQCollator() {
    _collator.setCaseSensitivity(Qt::CaseSensitive);
    _collator.setNumericMode(true);
}

bool NaturalSorterQCollator::compare(const QString &l, const QString &r, SortingOptions /*options*/) const {
    // Fix for the new breaking changes in QCollator in Qt 5.14 - null strings are no longer a valid input
    return _collator.compare(qToStringViewIgnoringNull(l), qToStringViewIgnoringNull(r)) < 0;
}
