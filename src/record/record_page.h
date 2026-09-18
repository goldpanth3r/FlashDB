#ifndef FLASHDB_RECORD_RECORD_PAGE_H
#define FLASHDB_RECORD_RECORD_PAGE_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>

#include "file/page.h"
#include "record/layout.h"

namespace flashdb {

class RecordPage {
public:
    RecordPage(Page& page, const Layout& layout);

    std::size_t insert(
        const std::unordered_map<std::string, std::string>& values
    );

    void insert_at(
        std::size_t slot,
        const std::unordered_map<std::string, std::string>& values
    );

    std::string get(
        std::size_t slot,
        const std::string& field_name
    ) const;

    void set(
        std::size_t slot,
        const std::string& field_name,
        const std::string& value
    );

    void remove(std::size_t slot);

    bool is_used(std::size_t slot) const;

    std::size_t slot_count() const;

    bool has_free_slot() const;

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