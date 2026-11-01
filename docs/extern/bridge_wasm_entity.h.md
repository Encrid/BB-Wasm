# include/bridge_wasm_entity.h

This header reads entities: the id at a row, and the location of an id.
It calls the accessors, which reach the entity index and the who-array of each table.

## Functions

| Function | Result |
| --- | --- |
| `bridge_get_entity` | Gives the entity at a table and a row. |
| `bridge_get_location` | Gives the location of an id: its table and row, or all ones if it is dead. |
| `bridge_is_alive` | Gives whether an id still names a live entity. |
| `bridge_get_entity_table` | Gives the table of a packed location. |
| `bridge_get_entity_row` | Gives the row of a packed location. |
