#include "rid.h"

namespace flashdb {

/**
 * RecordId creates an identifier for one record.
 *
 * @param page_number Page containing the record.
 * @param slot_number Slot containing the record.
 */
RecordId::RecordId(
    int page_number,
    std::size_t slot_number)
    : page_number_(page_number),
      slot_number_(slot_number) {
}

/**
 * page_number returns the page containing the record.
 *
 * @return Page number.
 */
int RecordId::page_number() const {
    return page_number_;
}

/**
 * slot_number returns the slot containing the record.
 *
 * @return Slot number.
 */
std::size_t RecordId::slot_number() const {
    return slot_number_;
}

/**
 * operator== compares two record identifiers.
 *
 * @param other RecordId to compare.
 * @return true if both identifiers point to the same record.
 */
bool RecordId::operator==(const RecordId& other) const {
    return page_number_ == other.page_number_
        && slot_number_ == other.slot_number_;
}

} // namespace flashdb