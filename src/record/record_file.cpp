#include "record_file.h"

#include <limits>
#include <stdexcept>

#include "record/record_page.h"

namespace flashdb {

/**
 * RecordFile opens or creates a table file.
 *
 * @param file_manager File manager used to create and inspect pages.
 * @param buffer_manager Buffer manager used to access pages in memory.
 * @param table_name Name of the table.
 * @param layout Layout describing records.
 */
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

    // Every record needs one byte for the used flag.
    const std::size_t slot_size =
        1 + layout_.record_size();

    if (slot_size > Page::PAGE_SIZE) {
        throw std::invalid_argument(
            "RecordFile: record is too large for one page"
        );
    }
}

/**
 * table_filename returns the file used to store this table.
 *
 * @return Table filename.
 */
std::string RecordFile::table_filename() const {
    return table_name_ + ".tbl";
}

/**
 * block_id creates the BlockId for one table page.
 *
 * @param page_number Page number inside the table.
 * @return BlockId identifying the page.
 */
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

/**
 * validate_page_number checks that a page exists in the table.
 *
 * @param page_number Page number to validate.
 */
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

/**
 * insert adds a record to the table.
 *
 * The table is treated as a heap file.
 *
 * The pages are searched from the beginning until a page
 * with a free slot is found.
 *
 * If all existing pages are full, a new page is appended.
 *
 * @param values Field values for the new record.
 * @return RecordId identifying the inserted record.
 */
RecordId RecordFile::insert(
    const std::unordered_map<std::string, std::string>& values) {

    // Validate the record before creating or modifying any page.
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

    // If the table does not have any pages yet,
    // create the first page.
    if (page_count == 0) {

        file_manager_.append(filename);

        page_count = 1;
    }

    // Search existing pages for a free slot.
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

    // Every existing page is full.
    //
    // Create one more page at the end of the table.
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

/**
 * get reads one field from a record.
 *
 * @param rid Record identifier.
 * @param field_name Field to read.
 * @return Field value.
 */
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

/**
 * set updates one field in a record.
 *
 * @param rid Record identifier.
 * @param field_name Field to update.
 * @param value New value.
 */
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

/**
 * remove deletes a record from the table.
 *
 * @param rid Record identifier.
 */
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

int RecordFile::page_count() const {
    return file_manager_.length(table_filename());
}

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

} // namespace flashdb