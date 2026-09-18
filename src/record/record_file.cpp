#include "record_file.h"

#include <limits>
#include <stdexcept>

#include "record/record_page.h"

namespace flashdb {

// Validate the table storage configuration before it is used.
RecordFile::RecordFile(
    FileManager& file_manager,
    BufferManager& buffer_manager,
    const std::string& table_name,
    const Layout& layout)
    : file_manager_(file_manager),
      buffer_manager_(buffer_manager),
      table_name_(table_name),
      layout_(layout) {

    if (table_name_.empty()) {
        throw std::invalid_argument(
            "RecordFile: table name cannot be empty"
        );
    }

    if (layout_.record_size() == 0) {
        throw std::invalid_argument(
            "RecordFile: record size cannot be zero"
        );
    }

    const std::size_t slot_size =
        1 + layout_.record_size();

    if (slot_size > Page::PAGE_SIZE) {
        throw std::invalid_argument(
            "RecordFile: record is too large for one page"
        );
    }
}

// Build the physical filename used by this table.
std::string RecordFile::table_filename() const {
    return table_name_ + ".tbl";
}

// Convert a logical page number into its physical database block.
BlockId RecordFile::block_id(
    int page_number) const {

    if (page_number < 0) {
        throw std::invalid_argument(
            "RecordFile::block_id: invalid page number"
        );
    }

    return BlockId(
        table_filename(),
        page_number
    );
}

// Make sure a requested table page actually exists.
void RecordFile::validate_page_number(
    int page_number) const {

    if (page_number < 0) {
        throw std::out_of_range(
            "RecordFile: invalid page number"
        );
    }

    const int page_count =
        file_manager_.length(table_filename());

    if (page_number >= page_count) {
        throw std::out_of_range(
            "RecordFile: page does not exist"
        );
    }
}

// Insert a new record into the first available table slot.
RecordId RecordFile::insert(
    const std::unordered_map<std::string, std::string>& values) {

    Page validation_page;

    RecordPage validator(
        validation_page,
        layout_
    );

    validator.validate_record(values);

    const std::string filename =
        table_filename();

    int page_count =
        file_manager_.length(filename);

    if (page_count == 0) {
        file_manager_.append(filename);
        page_count = 1;
    }

    for (int page_number = 0;
         page_number < page_count;
         ++page_number) {

        Buffer* buffer =
            buffer_manager_.get_buffer(
                block_id(page_number)
            );

        if (buffer == nullptr) {
            throw std::runtime_error(
                "RecordFile::insert: unable to get buffer"
            );
        }

        try {
            RecordPage record_page(
                buffer->page(),
                layout_
            );

            if (!record_page.has_free_slot()) {
                buffer_manager_.unpin_buffer(*buffer);
                continue;
            }

            const std::size_t slot =
                record_page.insert(values);

            buffer->mark_dirty();

            buffer_manager_.unpin_buffer(*buffer);

            return RecordId(
                page_number,
                slot
            );

        } catch (...) {
            buffer_manager_.unpin_buffer(*buffer);
            throw;
        }
    }

    const BlockId new_block =
        file_manager_.append(filename);

    const int new_page_number =
        page_count;

    Buffer* buffer =
        buffer_manager_.get_buffer(new_block);

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::insert: unable to get new page buffer"
        );
    }

    try {
        RecordPage record_page(
            buffer->page(),
            layout_
        );

        const std::size_t slot =
            record_page.insert(values);

        buffer->mark_dirty();

        buffer_manager_.unpin_buffer(*buffer);

        return RecordId(
            new_page_number,
            slot
        );

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Read one field from a stored record.
std::string RecordFile::get(
    const RecordId& rid,
    const std::string& field_name) {

    validate_page_number(
        rid.page_number()
    );

    Buffer* buffer =
        buffer_manager_.get_buffer(
            block_id(rid.page_number())
        );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::get: unable to get buffer"
        );
    }

    try {
        RecordPage record_page(
            buffer->page(),
            layout_
        );

        const std::string value =
            record_page.get(
                rid.slot_number(),
                field_name
            );

        buffer_manager_.unpin_buffer(*buffer);

        return value;

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Update one field while keeping the physical record in place.
void RecordFile::set(
    const RecordId& rid,
    const std::string& field_name,
    const std::string& value) {

    validate_page_number(
        rid.page_number()
    );

    Buffer* buffer =
        buffer_manager_.get_buffer(
            block_id(rid.page_number())
        );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::set: unable to get buffer"
        );
    }

    try {
        RecordPage record_page(
            buffer->page(),
            layout_
        );

        record_page.set(
            rid.slot_number(),
            field_name,
            value
        );

        buffer->mark_dirty();

        buffer_manager_.unpin_buffer(*buffer);

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Mark an existing record slot as unused.
void RecordFile::remove(
    const RecordId& rid) {

    validate_page_number(
        rid.page_number()
    );

    Buffer* buffer =
        buffer_manager_.get_buffer(
            block_id(rid.page_number())
        );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::remove: unable to get buffer"
        );
    }

    try {
        RecordPage record_page(
            buffer->page(),
            layout_
        );

        record_page.remove(
            rid.slot_number()
        );

        buffer->mark_dirty();

        buffer_manager_.unpin_buffer(*buffer);

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Read every field so a complete record can be saved for undo.
std::unordered_map<std::string, std::string>
RecordFile::get_record(
    const RecordId& rid) {

    validate_page_number(
        rid.page_number()
    );

    Buffer* buffer =
        buffer_manager_.get_buffer(
            block_id(rid.page_number())
        );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::get_record: unable to get buffer"
        );
    }

    try {
        RecordPage record_page(
            buffer->page(),
            layout_
        );

        std::unordered_map<std::string, std::string> values;

        for (const Field& field :
             layout_.schema().fields()) {

            values[field.name] =
                record_page.get(
                    rid.slot_number(),
                    field.name
                );
        }

        buffer_manager_.unpin_buffer(*buffer);

        return values;

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Restore a deleted record into its original physical slot.
void RecordFile::restore(
    const RecordId& rid,
    const std::unordered_map<std::string, std::string>& values) {

    validate_page_number(
        rid.page_number()
    );

    Buffer* buffer =
        buffer_manager_.get_buffer(
            block_id(rid.page_number())
        );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::restore: unable to get buffer"
        );
    }

    try {
        RecordPage record_page(
            buffer->page(),
            layout_
        );

        if (record_page.is_used(
                rid.slot_number())) {

            throw std::runtime_error(
                "RecordFile::restore: slot is already occupied"
            );
        }

        record_page.validate_record(values);

        const std::size_t offset =
            rid.slot_number() *
            (1 + layout_.record_size());

        buffer->page().data()[offset] =
            std::byte{1};

        for (const Field& field :
             layout_.schema().fields()) {

            record_page.set(
                rid.slot_number(),
                field.name,
                values.at(field.name)
            );
        }

        buffer->mark_dirty();

        buffer_manager_.unpin_buffer(*buffer);

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Associate the changed page with the WAL record that describes it.
void RecordFile::set_log_sequence_number(
    const RecordId& rid,
    std::size_t lsn) {

    validate_page_number(
        rid.page_number()
    );

    Buffer* buffer =
        buffer_manager_.get_buffer(
            block_id(rid.page_number())
        );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "RecordFile::set_log_sequence_number: "
            "unable to get buffer"
        );
    }

    try {
        buffer_manager_.set_log_sequence_number(
            *buffer,
            lsn
        );

        buffer_manager_.unpin_buffer(*buffer);

    } catch (...) {
        buffer_manager_.unpin_buffer(*buffer);
        throw;
    }
}

// Return the number of physical pages belonging to the table.
int RecordFile::page_count() const {
    return file_manager_.length(
        table_filename()
    );
}

// Collect the identifiers of all currently occupied slots.
std::vector<RecordId> RecordFile::scan() {
    std::vector<RecordId> records;

    const int pages = page_count();

    for (int page_number = 0;
         page_number < pages;
         ++page_number) {

        Buffer* buffer =
            buffer_manager_.get_buffer(
                block_id(page_number)
            );

        if (buffer == nullptr) {
            throw std::runtime_error(
                "RecordFile::scan: unable to get buffer"
            );
        }

        try {
            RecordPage record_page(
                buffer->page(),
                layout_
            );

            for (std::size_t slot = 0;
                 slot < record_page.slot_count();
                 ++slot) {

                if (record_page.is_used(slot)) {
                    records.emplace_back(
                        page_number,
                        slot
                    );
                }
            }

            buffer_manager_.unpin_buffer(*buffer);

        } catch (...) {
            buffer_manager_.unpin_buffer(*buffer);
            throw;
        }
    }

    return records;
}

// Expose the physical layout used by the table storage.
const Layout& RecordFile::layout() const {
    return layout_;
}

// Identify the table represented by this record file.
const std::string& RecordFile::table_name() const {
    return table_name_;
}

// Find the first physical slot that can accept a new record.
RecordId RecordFile::find_insert_rid() const {
    const int pages = page_count();

    for (int page_number = 0; page_number < pages; ++page_number) {
        Buffer* buffer = buffer_manager_.get_buffer(
            block_id(page_number)
        );

        if (buffer == nullptr) {
            throw std::runtime_error(
                "No available buffer for record insertion"
            );
        }

        RecordPage record_page(
            buffer->page(),
            layout_
        );

        for (std::size_t slot = 0;
             slot < record_page.slot_count();
             ++slot) {
            if (!record_page.is_used(slot)) {
                buffer_manager_.unpin_buffer(*buffer);
                return RecordId(page_number, slot);
            }
        }

        buffer_manager_.unpin_buffer(*buffer);
    }

    return RecordId(
        pages,
        0
    );
}

// Insert a record at a predetermined slot so the transaction can log the RID first.
// Insert a record at a predetermined slot so the transaction can log the RID first.
void RecordFile::insert_at(
    const RecordId& rid,
    const std::unordered_map<std::string, std::string>& values,
    std::size_t log_sequence_number
) {
    if (rid.page_number() < 0) {
        throw std::runtime_error("Invalid record page");
    }

    const int pages = page_count();

    if (rid.page_number() > pages) {
        throw std::runtime_error("Invalid insertion page");
    }

    if (rid.page_number() == pages) {
        file_manager_.append(table_filename());
    }

    Buffer* buffer = buffer_manager_.get_buffer(
        block_id(rid.page_number())
    );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "No available buffer for record insertion"
        );
    }

    RecordPage record_page(
        buffer->page(),
        layout_
    );

    if (rid.slot_number() >= record_page.slot_count()) {
        buffer_manager_.unpin_buffer(*buffer);
        throw std::runtime_error("Invalid record slot");
    }

    buffer_manager_.set_log_sequence_number(
        *buffer,
        log_sequence_number
    );

    record_page.insert_at(
        rid.slot_number(),
        values
    );

    buffer->mark_dirty();

    buffer_manager_.unpin_buffer(*buffer);
}

// Update a record while associating the page with its WAL record.
void RecordFile::set_with_log(
    const RecordId& rid,
    const std::string& field_name,
    const std::string& value,
    std::size_t log_sequence_number
) {
    validate_page_number(rid.page_number());

    Buffer* buffer = buffer_manager_.get_buffer(
        block_id(rid.page_number())
    );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "No available buffer for record update"
        );
    }

    buffer_manager_.set_log_sequence_number(
        *buffer,
        log_sequence_number
    );

    RecordPage record_page(
        buffer->page(),
        layout_
    );

    record_page.set(
        rid.slot_number(),
        field_name,
        value
    );

    buffer->mark_dirty();

    buffer_manager_.unpin_buffer(*buffer);
}

// Remove a record while associating the page with its WAL record.
void RecordFile::remove_with_log(
    const RecordId& rid,
    std::size_t log_sequence_number
) {
    validate_page_number(rid.page_number());

    Buffer* buffer = buffer_manager_.get_buffer(
        block_id(rid.page_number())
    );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "No available buffer for record deletion"
        );
    }

    buffer_manager_.set_log_sequence_number(
        *buffer,
        log_sequence_number
    );

    RecordPage record_page(
        buffer->page(),
        layout_
    );

    record_page.remove(
        rid.slot_number()
    );

    buffer->mark_dirty();

    buffer_manager_.unpin_buffer(*buffer);
}

// Restore a deleted record into its original physical slot.
void RecordFile::restore(
    const RecordId& rid,
    const std::unordered_map<std::string, std::string>& values,
    std::size_t log_sequence_number
) {
    validate_page_number(rid.page_number());

    Buffer* buffer = buffer_manager_.get_buffer(
        block_id(rid.page_number())
    );

    if (buffer == nullptr) {
        throw std::runtime_error(
            "No available buffer for record restore"
        );
    }

    RecordPage record_page(
        buffer->page(),
        layout_
    );

    if (record_page.is_used(rid.slot_number())) {
        buffer_manager_.unpin_buffer(*buffer);
        throw std::runtime_error(
            "Cannot restore an occupied record slot"
        );
    }

    buffer_manager_.set_log_sequence_number(
        *buffer,
        log_sequence_number
    );

    record_page.insert_at(
        rid.slot_number(),
        values
    );

    buffer->mark_dirty();

    buffer_manager_.unpin_buffer(*buffer);
}

} // namespace flashdb