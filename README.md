# FlashDB

**FlashDB is my attempt to build a relational SQL database from scratch in C++.**

I’m building it to understand how databases actually work internally instead of treating a database as a black box.

The architecture is inspired by [Edward Sciore’s *Database Design and Implementation*](https://www.goodreads.com/book/show/50660727-database-design-and-implementation).

## Architecture

```text
SQL
 ↓
Lexer
 ↓
Parser
 ↓
AST
 ↓
Planner
 ↓
Executor
 ↓
Record Manager
 ↓
Buffer Manager
 ↓
File Manager
 ↓
Disk
```

## Development

### Storage

* **File Manager** — manages database files and fixed-size pages.
* **Buffer Manager** — caches pages in memory and tracks dirty and pinned pages.
* **Record Manager** — stores, reads, updates, and deletes records.

### Metadata

* **Schema** — defines table columns and data types.
* **Table Catalog** — manages table definitions.
* **Record Layout** — manages field positions and record sizes.

### SQL

The SQL frontend converts SQL text into tokens, parses them, and builds an AST representing each query.

#### Supported Operations

```sql
-- Create a table
CREATE TABLE student (
    id INT,
    name VARCHAR
);

-- Insert a row
INSERT INTO student VALUES (1, 'Alice');

-- Read rows
SELECT name
FROM student;

-- Filter rows
SELECT name
FROM student
WHERE id = 1;

-- Update rows
UPDATE student
SET name = 'Bob'
WHERE id = 1;

-- Delete rows
DELETE FROM student
WHERE id = 1;
```

#### Keyword Groups

**DDL — Data Definition**

```text
CREATE
TABLE
```

**DML — Data Manipulation**

```text
INSERT
INTO
VALUES
UPDATE
SET
DELETE
```

**Query**

```text
SELECT
FROM
WHERE
```

**Data Types**

```text
INT
VARCHAR
```

**Supported Operators**

```text
=
```

#### Current Limitations

* No joins
* No `ORDER BY`
* No `GROUP BY`
* No aggregate functions
* No subqueries
* No index-based query execution
* Limited data types
* Limited expression operators

### Query Engine

* **Planner** — converts SQL ASTs into query plans.
* **Executor** — executes query plans.
* **Operators** — provides scans, filters, and projections.

### Transactions

* **Transactions** — groups database operations into units of work.
* **Logging** — records changes for durability.
* **Recovery** — restores database state after failures.

### Indexing

* **B+ Tree** — planned index structure for efficient lookups.
* **Index Manager** — manages indexes associated with tables.
* **Index Scans** — enables indexed query execution.

### Optimization

* **Query Optimizer** — chooses execution plans.
* **Cost Estimation** — estimates plan costs.
* **Plan Selection** — selects an execution strategy.

## Project Structure

```text
FlashDB/
├── src/
│   ├── file/
│   ├── log/
│   ├── buffer/
│   ├── tx/
│   ├── record/
│   ├── metadata/
│   ├── parser/
│   ├── planner/
│   ├── query/
│   └── index/
├── tests/
├── docs/
├── CMakeLists.txt
└── README.md
```

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Test

```bash
ctest --test-dir build --output-on-failure
```

## Project Goals

FlashDB was initially built as an educational project to understand how SQL and database systems work internally. It has since grown into a working database engine that closely follows the structure and behavior of a real database system.
