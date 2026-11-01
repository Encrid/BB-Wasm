# include/bridge_wasm_span.h

This header reads a query's spans from the query list in the private header.
A span is one matched table: its id, 4 bytes.
The row count of each table is in the table directory of the shared memory, which Bridge writes once at each barrier.

## Functions

| Function | Result |
| --- | --- |
| `bridge_count_spans` | Gives how many spans a query matched. |
| `bridge_get_span` | Gives the address of span `i` of a query. |
| `bridge_span_table` | Gives the id of the table that a span names. |
| `bridge_count_rows` | Gives how many rows the table of a span has, through `acc_rows`. |
