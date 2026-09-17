#ifndef FLASHDB_RECORD_RECORD_PAGE_H
#define FLASHDB_RECORD_RECORD_PAGE_H

#include <cstddef>
#include <string>

#include "file/page.h"
#include "record/schema.h"

namespace flashdb {

/**
 * RecordPage manages database records inside one Page.
 *
 * Each record has the same layout defined by a Schema.
 */
class RecordPage {
public:
    /**
     * Create a record page using an existing page.
     *
     * @param page Page containing the records.
     * @param schema Schema describing each record.
     */
    RecordPage(Page& page, const Schema& schema);

    /**
     * Insert a record into the page.
     *
     * @param values Serialized field values.
     * @return Record slot number.
     */
    int insert(const std::string& values);

    /**
     * Read a record from the page.
     *
     * @param slot Record slot number.
     * @return Stored record.
     */
    std::string get(int slot) const;

private:
    Page& page_;
    const Schema& schema_;
    std::size_t record_size_;

    /**
     * Calculate the size of one record.
     */
    std::size_t calculate_record_size() const;
};

} // namespace flashdb

#endif