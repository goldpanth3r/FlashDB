#include "record_page.h"

#include <cstring>
#include <stdexcept>

namespace flashdb {

RecordPage::RecordPage(
    Page& page,
    const Layout& layout)
    : page_(page),
      layout_(layout) {
}

/**
 * slot_offset calculates where a record slot begins.
 *
 * @param slot Record slot number.
 * @return Byte offset of the slot.
 */
std::size_t RecordPage::slot_offset(
    std::size_t slot) const {

    return slot * (
        USED_FLAG_SIZE + layout_.record_size()
    );
}

/**
 * validate_slot checks that a slot exists in this page.
 *
 * @param slot Slot number to validate.
 */
void RecordPage::validate_slot(
    std::size_t slot) const {

    if (slot >= slot_count()) {
        throw std::out_of_range(
            "RecordPage: slot outside page"
        );
    }
}

/**
 * validate_field_value checks that a value fits its field.
 *
 * @param field_name Field being written.
 * @param value Value being written.
 */
void RecordPage::validate_field_value(
    const std::string& field_name,
    const std::string& value) const {

    const Field* field = nullptr;

    for (const Field& current : layout_.schema().fields()) {
        if (current.name == field_name) {
            field = &current;
            break;
        }
    }

    if (field == nullptr) {
        throw std::out_of_range(
            "RecordPage: unknown field: " + field_name
        );
    }

    if (field->type == FieldType::STRING &&
        value.size() > field->length) {

        throw std::length_error(
            "RecordPage: string value is too large"
        );
    }

    if (field->type == FieldType::INT) {
        try {
            std::size_t position = 0;

            std::stoi(value, &position);

            if (position != value.size()) {
                throw std::invalid_argument(
                    "invalid integer"
                );
            }
        } catch (...) {
            throw std::invalid_argument(
                "RecordPage: invalid integer value"
            );
        }
    }
}

/**
 * slot_count calculates how many complete records fit in the page.
 *
 * @return Number of record slots.
 */
std::size_t RecordPage::slot_count() const {

    const std::size_t slot_size =
        USED_FLAG_SIZE + layout_.record_size();

    if (slot_size == 0) {
        return 0;
    }

    return Page::PAGE_SIZE / slot_size;
}

/**
 * is_used checks whether a record exists in a slot.
 *
 * @param slot Slot number.
 * @return true if the slot contains a record.
 */
bool RecordPage::is_used(
    std::size_t slot) const {

    validate_slot(slot);

    const std::size_t offset =
        slot_offset(slot);

    return page_.data()[offset] != std::byte{0};
}

void RecordPage::validate_record(
    const std::unordered_map<std::string, std::string>& values
) const {

    for (const Field& field :
         layout_.schema().fields()) {

        const auto it =
            values.find(field.name);

        if (it == values.end()) {
            throw std::invalid_argument(
                "RecordPage::validate_record: missing field: "
                + field.name
            );
        }

        validate_field_value(
            field.name,
            it->second
        );
    }
}

/**
 * insert stores a new record in the first free slot.
 *
 * All field values are validated before changing the page.
 * This prevents a failed insert from leaving a partially-created record.
 *
 * @param values Field values for the record.
 * @return Slot containing the inserted record.
 */
std::size_t RecordPage::insert(
    const std::unordered_map<std::string, std::string>& values) {

    // Find the first free slot.
    for (std::size_t slot = 0;
         slot < slot_count();
         ++slot) {

        if (is_used(slot)) {
            continue;
        }

        // Step 1:
        // Check that every field exists and every value is valid.
        //
        // We do this before modifying the page.
                validate_record(values);

        // Step 2:
        // All validation succeeded.
        // Now mark the slot as occupied.
        const std::size_t offset =
            slot_offset(slot);

        page_.data()[offset] = std::byte{1};

        // Step 3:
        // Store every field.
        for (const Field& field :
             layout_.schema().fields()) {

            set(
                slot,
                field.name,
                values.at(field.name)
            );
        }

        return slot;
    }

    throw std::runtime_error(
        "RecordPage::insert: page is full"
    );
}

/**
 * get reads one field from a record.
 *
 * @param slot Slot containing the record.
 * @param field_name Field to read.
 * @return Field value.
 */
std::string RecordPage::get(
    std::size_t slot,
    const std::string& field_name) const {

    validate_slot(slot);

    if (!is_used(slot)) {
        throw std::runtime_error(
            "RecordPage::get: slot is empty"
        );
    }

    const std::size_t field_offset =
        layout_.offset(field_name);

    const std::size_t offset =
        slot_offset(slot)
        + USED_FLAG_SIZE
        + field_offset;

    const Field* field = nullptr;

    for (const Field& current :
         layout_.schema().fields()) {

        if (current.name == field_name) {
            field = &current;
            break;
        }
    }

    if (field == nullptr) {
        throw std::out_of_range(
            "RecordPage::get: unknown field"
        );
    }

    if (field->type == FieldType::INT) {

        return std::to_string(
            page_.get_int(offset)
        );
    }

    if (field->type == FieldType::STRING) {

        const auto& data = page_.data();

        const char* chars =
            reinterpret_cast<const char*>(
                data.data() + offset
            );

        std::size_t length = 0;

        while (length < field->length &&
               chars[length] != '\0') {
            ++length;
        }

        return std::string(chars, length);
    }

    throw std::runtime_error(
        "RecordPage::get: unsupported field type"
    );
}

/**
 * set updates one field in a record.
 *
 * @param slot Slot containing the record.
 * @param field_name Field to update.
 * @param value New value.
 */
void RecordPage::set(
    std::size_t slot,
    const std::string& field_name,
    const std::string& value) {

    validate_slot(slot);

    if (!is_used(slot)) {
        throw std::runtime_error(
            "RecordPage::set: slot is empty"
        );
    }

    validate_field_value(
        field_name,
        value
    );

    const std::size_t offset =
        slot_offset(slot)
        + USED_FLAG_SIZE
        + layout_.offset(field_name);

    const Field* field = nullptr;

    for (const Field& current :
         layout_.schema().fields()) {

        if (current.name == field_name) {
            field = &current;
            break;
        }
    }

    if (field == nullptr) {
        throw std::out_of_range(
            "RecordPage::set: unknown field"
        );
    }

    if (field->type == FieldType::INT) {

        page_.set_int(
            offset,
            std::stoi(value)
        );

        return;
    }

    if (field->type == FieldType::STRING) {

        auto& data = page_.data();

        std::memset(
            data.data() + offset,
            0,
            field->length
        );

        std::memcpy(
            data.data() + offset,
            value.data(),
            value.size()
        );

        return;
    }

    throw std::runtime_error(
        "RecordPage::set: unsupported field type"
    );
}

/**
 * remove deletes a record from a slot.
 *
 * @param slot Slot containing the record.
 */
void RecordPage::remove(
    std::size_t slot) {

    validate_slot(slot);

    if (!is_used(slot)) {
        return;
    }

    const std::size_t offset =
        slot_offset(slot);

    page_.data()[offset] = std::byte{0};

    std::memset(
        page_.data().data()
            + offset
            + USED_FLAG_SIZE,
        0,
        layout_.record_size()
    );
}

bool RecordPage::has_free_slot() const {
    for (std::size_t slot = 0;
         slot < slot_count();
         ++slot) {

        if (!is_used(slot)) {
            return true;
        }
    }

    return false;
}

// Place a new record into a specific physical slot.
void RecordPage::insert_at(
    std::size_t slot,
    const std::unordered_map<std::string, std::string>& values
) {
    validate_slot(slot);

    if (is_used(slot)) {
        throw std::runtime_error("Record slot is already occupied");
    }

    validate_record(values);

    const std::size_t offset = slot_offset(slot);

    page_.data()[offset] = std::byte{1};

    for (const auto& field : layout_.schema().fields()) {
        set(
            slot,
            field.name,
            values.at(field.name)
        );
    }
}

} // namespace flashdb