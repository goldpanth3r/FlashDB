#ifndef FLASHDB_RECORD_RECORD_PAGE_H
#define FLASHDB_RECORD_RECORD_PAGE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "file/page.h"
#include "record/layout.h"

namespace flashdb {

/**
 * RecordPage stores fixed-size records inside one database page.
 *
 * Each slot contains:
 *
 * [1 byte used flag][record data]
 *
 * A used flag of 0 means the slot is free.
 * A used flag of 1 means the slot contains a record.
 */
class RecordPage {
public:
    /**
     * Create a record page using the supplied layout.
     *
     * @param page Database page containing records.
     * @param layout Layout describing each record.
     */
    RecordPage(Page& page, const Layout& layout);

    /**
     * Insert a record into the first available slot.
     *
     * @param values Values for the record fields.
     * @return Slot number containing the new record.
     */
    std::size_t insert(
        const std::unordered_map<std::string, std::string>& values
    );

    /**
     * Read a field from a record.
     *
     * @param slot Slot containing the record.
     * @param field_name Field to read.
     * @return Field value as a string.
     */
    std::string get(
        std::size_t slot,
        const std::string& field_name
    ) const;

    /**
     * Update one field in a record.
     *
     * @param slot Slot containing the record.
     * @param field_name Field to update.
     * @param value New field value.
     */
    void set(
        std::size_t slot,
        const std::string& field_name,
        const std::string& value
    );

    /**
     * Delete a record from a slot.
     *
     * @param slot Slot containing the record.
     */
    void remove(std::size_t slot);

    /**
     * Check whether a slot contains a record.
     *
     * @param slot Slot number.
     * @return true if the slot is occupied.
     */
    bool is_used(std::size_t slot) const;

    /**
     * Return the maximum number of record slots in the page.
     *
     * @return Number of slots.
     */
    std::size_t slot_count() const;

    /**
    * Check whether this page has at least one free slot.
    */
    bool has_free_slot() const;

    /**
        * Validate all values before inserting a record.
        */
        void validate_record(
            const std::unordered_map<std::string, std::string>& values
        ) const;

private:
    static constexpr std::size_t USED_FLAG_SIZE = 1;

    Page& page_;
    const Layout& layout_;

    std::size_t slot_offset(std::size_t slot) const;
    void validate_slot(std::size_t slot) const;
    void validate_field_value(
        const std::string& field_name,
        const std::string& value
    ) const;
};

} // namespace flashdb

#endif