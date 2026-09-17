#ifndef FLASHDB_RECORD_RID_H
#define FLASHDB_RECORD_RID_H

#include <cstddef>

namespace flashdb {

/**
 * RecordId identifies one record inside a database table.
 *
 * A record is identified by:
 *
 *     page number + slot number
 */
class RecordId {
public:
    /**
     * Create a record identifier.
     *
     * @param page_number Page containing the record.
     * @param slot_number Slot containing the record.
     */
    RecordId(
        int page_number,
        std::size_t slot_number
    );

    /**
     * Return the page number.
     *
     * @return Page number.
     */
    int page_number() const;

    /**
     * Return the slot number.
     *
     * @return Slot number.
     */
    std::size_t slot_number() const;

    /**
     * Compare two record identifiers.
     *
     * @param other RecordId to compare.
     * @return true if both identifiers are equal.
     */
    bool operator==(const RecordId& other) const;

private:
    int page_number_;
    std::size_t slot_number_;
};

} // namespace flashdb

#endif