#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "database.h"
#include "log/log_manager.h"
#include "parser/lexer.h"
#include "parser/parser.h"
#include "planner/planner.h"
#include "query/create_table_executor.h"
#include "query/delete_executor.h"
#include "query/insert_executor.h"
#include "query/query_executor.h"
#include "query/update_executor.h"
#include "tx/transaction.h"
#include "recovery/recovery_manager.h"

int main() {
    try {
        flashdb::Database database("data");
        flashdb::LogManager log_manager("data");
        
        flashdb::RecoveryManager recovery(
            database,
            log_manager
        );

        recovery.recover();

        flashdb::Planner planner;

        std::unique_ptr<flashdb::Transaction> transaction;

        std::cout << "FlashDB starting...\n";
        std::cout << "Tables: "
                  << database.catalog().table_count()
                  << '\n';

        std::string sql;

        while (true) {
            std::cout << "flashdb> ";

            if (!std::getline(std::cin, sql)) {
                break;
            }

            if (sql.empty()) {
                continue;
            }

            if (sql == "exit" || sql == "quit") {
                if (transaction) {
                    transaction->rollback();
                }
                break;
            }

            try {
                flashdb::Lexer lexer(sql);
                flashdb::Parser parser(lexer.tokenize());

                const auto statement = parser.parse();
                auto plan = planner.create_plan(statement);

                const std::string& name = plan->get_name();

                if (name == "BeginTransaction") {
                    if (transaction) {
                        std::cout << "Transaction already active\n";
                    } else {
                        transaction = std::make_unique<flashdb::Transaction>(
                            database,
                            log_manager
                        );
                        std::cout << "Transaction started\n";
                    }
                    continue;
                }

                if (name == "CommitTransaction") {
                    if (!transaction) {
                        std::cout << "No active transaction\n";
                    } else {
                        transaction->commit();
                        transaction.reset();
                        std::cout << "Transaction committed\n";
                    }
                    continue;
                }

                if (name == "RollbackTransaction") {
                    if (!transaction) {
                        std::cout << "No active transaction\n";
                    } else {
                        transaction->rollback();
                        transaction.reset();
                        std::cout << "Transaction rolled back\n";
                    }
                    continue;
                }

                if (name == "CreateTable") {
                    flashdb::CreateTableExecutor executor(
                        *plan,
                        database
                    );

                    executor.execute();

                    std::cout << "Table created: "
                              << plan->get_table_name()
                              << '\n';
                    continue;
                }

                if (name == "Insert") {
                    if (transaction) {
                        flashdb::InsertExecutor executor(
                            *plan,
                            database,
                            *transaction
                        );

                        const auto rid = executor.execute();

                        std::cout << "Inserted record at page "
                                  << rid.page_number()
                                  << ", slot "
                                  << rid.slot_number()
                                  << '\n';
                    } else {
                        flashdb::Transaction tx(
                            database,
                            log_manager
                        );

                        flashdb::InsertExecutor executor(
                            *plan,
                            database,
                            tx
                        );

                        const auto rid = executor.execute();
                        tx.commit();

                        std::cout << "Inserted record at page "
                                  << rid.page_number()
                                  << ", slot "
                                  << rid.slot_number()
                                  << '\n';
                    }

                    continue;
                }

                if (name == "Update") {
                    std::size_t count;

                    if (transaction) {
                        flashdb::UpdateExecutor executor(
                            *plan,
                            database,
                            *transaction
                        );
                        count = executor.execute();
                    } else {
                        flashdb::Transaction tx(
                            database,
                            log_manager
                        );

                        flashdb::UpdateExecutor executor(
                            *plan,
                            database,
                            tx
                        );

                        count = executor.execute();
                        tx.commit();
                    }

                    std::cout << "Updated "
                              << count
                              << " record(s)\n";
                    continue;
                }

                if (name == "Delete") {
                    std::size_t count;

                    if (transaction) {
                        flashdb::DeleteExecutor executor(
                            *plan,
                            database,
                            *transaction
                        );
                        count = executor.execute();
                    } else {
                        flashdb::Transaction tx(
                            database,
                            log_manager
                        );

                        flashdb::DeleteExecutor executor(
                            *plan,
                            database,
                            tx
                        );

                        count = executor.execute();
                        tx.commit();
                    }

                    std::cout << "Deleted "
                              << count
                              << " record(s)\n";
                    continue;
                }

                if (name == "Project") {
                    flashdb::QueryExecutor executor(
                        *plan,
                        database
                    );

                    const auto rows = executor.execute();

                    for (const auto& row : rows) {
                        for (const auto& value : row) {
                            std::cout << value << ' ';
                        }
                        std::cout << '\n';
                    }

                    continue;
                }

                std::cout << "Unsupported SQL statement\n";
            }
            catch (const std::exception& error) {
                if (transaction) {
                    transaction->rollback();
                    transaction.reset();
                    std::cout << "Transaction rolled back\n";
                }

                std::cerr << "Error: "
                          << error.what()
                          << '\n';
            }
        }

        std::cout << "FlashDB shutting down...\n";
        return 0;
    }
    catch (const std::exception& error) {
        std::cerr << "FlashDB error: "
                  << error.what()
                  << '\n';
        return 1;
    }
}