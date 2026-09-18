#pragma once

#include "record/rid.h"

namespace flashdb {

class RecordExecutor {
public:
    virtual ~RecordExecutor() = default;

    // open starts the executor.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    virtual void open() = 0;

    // has_next checks whether another record exists.
    // Arguments:
    // None.
    // Returns:
    // True if another record exists, otherwise false.
    virtual bool has_next() = 0;

    // next returns the next record.
    // Arguments:
    // None.
    // Returns:
    // The next record identifier.
    virtual RecordId next() = 0;

    // close stops the executor.
    // Arguments:
    // None.
    // Returns:
    // Nothing.
    virtual void close() = 0;
};

} // namespace flashdb