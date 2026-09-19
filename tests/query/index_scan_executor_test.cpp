#include <gtest/gtest.h>

#include "query/index_scan_executor.h"

namespace flashdb {

TEST(IndexScanExecutorTest, ReturnsIndexedRecords) {

    const RecordId first(0, 1);
    const RecordId second(0, 2);

    IndexScanExecutor executor({
        first,
        second
    });

    executor.open();

    ASSERT_TRUE(
        executor.has_next()
    );

    EXPECT_EQ(
        executor.next(),
        first
    );

    ASSERT_TRUE(
        executor.has_next()
    );

    EXPECT_EQ(
        executor.next(),
        second
    );

    EXPECT_FALSE(
        executor.has_next()
    );

    executor.close();
}

TEST(IndexScanExecutorTest, EmptyIndexReturnsNoRecords) {

    IndexScanExecutor executor({});

    executor.open();

    EXPECT_FALSE(
        executor.has_next()
    );

    executor.close();
}

} // namespace flashdb