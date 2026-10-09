# The tests

What BB-Wasm exposes, and the case for each. A case is a process: `python build.py --bridge DIR
test -k NAME` runs those whose name has NAME in it. A case is named for what it is of, a
hyphen, and what it shows, and is listed under that heading. `tools/cover.py` reads the
headers of the SDK, this document and the cases of `build.py`, and fails on an exposed name
with no heading, a heading with no case, and a case under no heading or the wrong one.

## The SDK

What a module is written with: `bridge_wasm.h`, and what it includes.

### BRIDGE_DECLARE

    BRIDGE_DECLARE-an-enum-and-a-bitmask

### BRIDGE_COMPONENT

    BRIDGE_COMPONENT-declared-by-two-modules

### BRIDGE_DECLARE_TAG

    BRIDGE_DECLARE_TAG-carried-by-two-of-three

### BRIDGE_ADD_SYSTEM

    BRIDGE_ADD_SYSTEM-registered-and-run

### BRIDGE_OBSERVER

    BRIDGE_OBSERVER-registered-and-told

### BRIDGE_PUT_CHANGES

    BRIDGE_PUT_CHANGES-an-observer-that-attaches

### BRIDGE_ATTACH

    BRIDGE_ATTACH-the-value-travels-with-it

### BRIDGE_ROW

    BRIDGE_ROW-no-call-in-the-loop
    BRIDGE_ROW-checked-and-unchecked-timed

### BRIDGE_GET

    BRIDGE_GET-a-field-read

### BRIDGE_SET

    BRIDGE_SET-a-field-written-each-frame

### BRIDGE_IMPORT

    BRIDGE_IMPORT-the-entries-imported

### BRIDGE_ACC

    BRIDGE_ACC-the-accessors-merged-in

### BRIDGE_EXPORT

    BRIDGE_EXPORT-a-system-found-by-its-name

### BRIDGE_HEADER

    BRIDGE_HEADER-the-scratch-arena-found

### bridge_get_entity

    bridge_get_entity-the-id-of-a-row

### bridge_get_ref

    bridge_get_ref-own-entities-followed

### bridge_is_alive

    bridge_is_alive-ids-a-sweep-left

### bridge_get_sparse_index

    bridge_get_sparse_index-where-the-index-is

### bridge_get_sparse_value

    bridge_get_sparse_value-the-value-of-an-id

### bridge_get_single

    bridge_get_single-where-the-one-row-is

### bridge_get_cells

    bridge_get_cells-where-the-cells-are

### bridge_get_cell

    bridge_get_cell-the-cell-of-an-id

### bridge_line_t

    bridge_line_t-a-line-of-text-found-at-run-time

### BRIDGE_LOG

    BRIDGE_LOG-a-line-with-two-numbers

### bridge_log

    bridge_log-a-line-of-text-found-at-run-time

### bridge_count_triggers

    bridge_count_triggers-how-many-this-barrier

### bridge_get_trigger

    bridge_get_trigger-each-in-turn

### bridge_get_trigger_id

    bridge_get_trigger_id-who-it-was

### bridge_get_trigger_events

    bridge_get_trigger_events-enter-attach-leave

### BRIDGE_STATE

    BRIDGE_STATE-a-statement-put-in-scratch

### bridge_take_scratch

    bridge_take_scratch-more-than-given-stops-the-module

### bridge_put

    bridge_put-bytes-copied-into-scratch

### bridge_lend

    bridge_lend-scratch-given-back

### bridge_count_spans

    bridge_count_spans-one-table-matched

### bridge_get_span

    bridge_get_span-one-table-walked

### bridge_count_rows

    bridge_count_rows-rows-of-a-span

### bridge_get_entities

    bridge_get_entities-where-a-span-keeps-its-ids

### bridge_get_base

    bridge_get_base-where-a-column-begins

### bridge_get_bits

    bridge_get_bits-where-a-toggle-keeps-them

### bridge_is_on

    bridge_is_on-counted-by-its-bit

### bridge_turn

    bridge_turn-four-turned-off

## The backend

The entries of Bridge's seam for a backend, as this one answers them.

### adopt

    adopt

### bridge_load

    bridge_load-loaded
    bridge_load-memory-with-a-data-segment
    bridge_load-init-trapped
    bridge_load-no-init-export
    bridge_load-failed-init-undone
    bridge_load-refused-leaves-nothing
    bridge_load-lifts-a-stopped-deck
    bridge_load-no-reload-export
    bridge_load-loaded-already-refused
    bridge_load-import-nobody-answers

### BRIDGE_NO_LIMIT

    BRIDGE_NO_LIMIT-stated-for-every-limit

### module_config.init_timeout

    module_config.init_timeout-init-interrupted
    module_config.init_timeout-unset-refused

### module_config.memory_limit

    module_config.memory_limit-growth-refused
    module_config.memory_limit-memory-larger-at-start
    module_config.memory_limit-unset-refused
    module_config.memory_limit-negative-refused

### instance_get_func

    instance_get_func-export-taking-an-argument-refused
    instance_get_func-export-answering-64-bits-refused

### module_config.abi

    module_config.abi-another-refused

### artifact.backend

    artifact.backend-not-configured-refused

### module_config.log_level

    module_config.log_level-unset-refused
    module_config.log_level-no-severity-refused
    module_config.log_level-warnings-drop-info

### BRIDGE_NONE

    BRIDGE_NONE-for-a-limit-refused

### config.abi

    config.abi-another-refused
    config.abi-unset-refused

### BRIDGE_UNSET

    BRIDGE_UNSET-flag-refused

### module_config.on_error

    module_config.on_error-fail-stops-and-skip-does-not
    module_config.on_error-unset-refused

### backend.library

    backend.library-cannot-open-refused

### bridge_create

    bridge_create-no-sink-refused
    bridge_create-budget-sum-beyond-32-bits-refused
    bridge_create-two-decks-at-once

### module_config.read

    module_config.read-another-namespace-read

### record.level

    record.level-info-with-its-two-numbers

### declare

    declare-storage-unset-refused
    declare-another-layout-refused
    declare-ungranted-namespace-refused
    declare-nested-types
    declare-undeclared-field-type-refused
    declare-statement-outside-scratch-refused
    declare-name-with-no-namespace-refused
    declare-kind-unset-refused
    declare-on-unload-unset-refused
    declare-size-other-than-descriptor-refused
    declare-outside-init-refused

### BRIDGE_DIRECT_INDEXED

    BRIDGE_DIRECT_INDEXED-reached-from-rows

### BRIDGE_TOGGLE

    BRIDGE_TOGGLE-counted-on-and-off

### BRIDGE_SPARSE_SET

    BRIDGE_SPARSE_SET-walked-and-reached

### add_system

    add_system-unknown-operator-refused
    add_system-own-conflict-in-registration-order
    add_system-statement-outside-scratch-refused
    add_system-more-changes-than-types-refused
    add_system-name-twice-refused
    add_system-change-of-an-undeclared-slot-refused
    add_system-change-ungranted-refused
    add_system-outside-init-refused

### module_config.work_limit

    module_config.work_limit-spinning-system-ended
    module_config.work_limit-unset-refused

### module_config.spawns

    module_config.spawns-used-up
    module_config.spawns-unset-refused
    module_config.spawns-more-than-entities-refused
    module_config.spawns-sum-beyond-entities-refused

### module_config.log_ring

    module_config.log_ring-full-lines-dropped
    module_config.log_ring-unset-refused
    module_config.log_ring-smaller-than-a-record-refused

### module_config.spans

    module_config.spans-too-small-refused
    module_config.spans-unset-refused
    module_config.spans-outgrown-at-a-barrier

### budget.columns

    budget.columns-used-up-changes-not-applied
    budget.columns-unset-refused
    budget.columns-no-room-for-a-singleton-refused

### bridge_run_phase

    bridge_run_phase-undeclared-phase-refused

### config.phases

    config.phases-none-refused
    config.phases-nameless-refused
    config.phases-twice-refused

### budget.tables

    budget.tables-unset-refused

### budget.types

    budget.types-unset-refused
    budget.types-reached-refused

### budget.systems

    budget.systems-unset-refused
    budget.systems-reached-refused

### budget.terms

    budget.terms-unset-refused
    budget.terms-exceeded-refused

### budget.entities

    budget.entities-beyond-32-bits-refused
    budget.entities-unset-refused

### module_config.scratch

    module_config.scratch-private-beyond-32-bits-refused
    module_config.scratch-unset-refused

### config.order

    config.order-mover-before-halver
    config.order-halver-before-mover
    config.order-cycle-refused

### order

    order-unordered-conflict-refused

### observe

    observe-chain-one-hop-a-barrier
    observe-reserved-name-refused
    observe-no-term-refused
    observe-statement-outside-scratch-refused
    observe-outside-init-refused

### config.threads

    config.threads-unset-refused
    config.threads-four-wasm-instances-hop

### alloc

    alloc-string-kept
    alloc-share-exhausted
    alloc-stated-as-none-refused

### module_config.persistent

    module_config.persistent-more-than-the-arena-refused
    module_config.persistent-unset-refused

### spawn

    spawn-stated-as-none-refused
    spawn-same-ids-on-one-thread-and-four

### BRIDGE_SKIP

    BRIDGE_SKIP-failed-init-skipped

### module_config.max_system_failures

    module_config.max_system_failures-reached-stops-the-module
    module_config.max_system_failures-one-in-three-forgiven
    module_config.max_system_failures-two-in-three-reached
    module_config.max_system_failures-clean-system-forgives-no-other
    module_config.max_system_failures-no-limit-never-stops
    module_config.max_system_failures-unset-refused

### BRIDGE_FAIL

    BRIDGE_FAIL-stopped-module-stops-the-deck
    BRIDGE_FAIL-refused-before-it-exists-stops-nothing

### bridge_unload

    bridge_unload-of-what-stopped-the-deck
    bridge_unload-declarer-goes-user-holds-the-type
    bridge_unload-not-loaded-refused
    bridge_unload-two-hundred-rounds
    bridge_unload-share-given-back
    bridge_unload-share-kept-while-pointed-into

### BRIDGE_STRIP

    BRIDGE_STRIP-unload-strips

### BRIDGE_SWEEP

    BRIDGE_SWEEP-unload-sweeps

### BRIDGE_DELETE

    BRIDGE_DELETE-unload-deletes

### module_config.shutdown_timeout

    module_config.shutdown_timeout-shutdown-interrupted
    module_config.shutdown_timeout-unset-refused

### budget.modules

    budget.modules-reached-refused
    budget.modules-unset-refused

### bridge_reload

    bridge_reload-larger-share
    bridge_reload-larger-share-past-a-neighbour
    bridge_reload-same-share-exhausted
    bridge_reload-smaller-share-refused
    bridge_reload-share-more-than-the-arena-refused
    bridge_reload-lifts-a-stopped-deck
    bridge_reload-stopped-module-revived
    bridge_reload-not-loaded-refused
    bridge_reload-settings-row-kept
    bridge_reload-config-dropped

### module_config.reloadable

    module_config.reloadable-false-with-hot-reload-refused
    module_config.reloadable-unset-refused

### module_config.settings

    module_config.settings-of-every-kind
    module_config.settings-text-beyond-the-share-refused
    module_config.settings-reseeded-in-place

### setting.key

    setting.key-no-such-field-refused

### setting.kind

    setting.kind-text-for-a-float-refused

### BRIDGE_STRING

    BRIDGE_STRING-longer-than-the-field-refused
    BRIDGE_STRING-no-such-constant-refused

### BRIDGE_INT

    BRIDGE_INT-beyond-the-field-refused
    BRIDGE_INT-beyond-the-enum-refused

### BRIDGE_NO_CONFIG

    BRIDGE_NO_CONFIG-with-settings-refused

### module_config.config

    module_config.config-unset-refused
    module_config.config-undeclared-refused
    module_config.config-taken-refused
    module_config.config-not-a-singleton-refused

### artifact.bytes

    artifact.bytes-none-for-wasm

### bridge_destroy

    bridge_destroy-shuts-down-what-is-loaded

### budget.persistent

    budget.persistent-unset-refused

### budget.sparse_sets

    budget.sparse_sets-unset-refused
    budget.sparse_sets-negative-refused
    budget.sparse_sets-beyond-32-bits-refused

### budget.direct_indexed

    budget.direct_indexed-unset-refused
    budget.direct_indexed-no-limit-refused
    budget.direct_indexed-beyond-32-bits-refused

### order.before

    order.before-none-refused

### order.after

    order.after-none-refused

### module_config.identity

    module_config.identity-none-refused

### module_config.trusted

    module_config.trusted-unset-refused

### module_config.hot_reload

    module_config.hot_reload-unset-refused

### module_config.reload_timeout

    module_config.reload_timeout-unset-refused

### module_config.changes

    module_config.changes-unset-refused
    module_config.changes-beyond-32-bits-refused

### module_config.losses

    module_config.losses-count-unset-refused
    module_config.losses-counted-and-none-refused

### phase_name.from

    phase_name.from-unset-refused

### module_config.phases

    module_config.phases-mapped-twice-refused

### bridge_declaration_t

    bridge_declaration_t-name-outside-scratch-refused

### BRIDGE_TAG

    BRIDGE_TAG-with-bytes-refused

### bridge_system_statement_t

    bridge_system_statement_t-phase-outside-scratch-refused
    bridge_system_statement_t-name-outside-scratch-refused

### bridge_observer_statement_t

    bridge_observer_statement_t-events-unset-refused

### bridge_term_t

    bridge_term_t-access-unset-refused
    bridge_term_t-access-neither-refused
    bridge_term_t-operator-unset-refused
    bridge_term_t-slot-undeclared-refused

### BRIDGE_WRITE

    BRIDGE_WRITE-ungranted-refused

### attach

    attach-undeclared-slot-refused
    attach-ungranted-refused
    attach-value-outside-scratch-refused
    attach-tag-with-a-value-refused

### detach

    detach-undeclared-slot-refused

### backend_sig_t

    backend_sig_t-an-i32-and-an-i64-answered

### backend_entry_t

    backend_entry_t-what-answers-an-import

### backend_memory_import_t

    backend_memory_import_t-one-block-in-two-stores

### memory_release

    memory_release-after-the-last-instance

### instance_create

    instance_create-one-block-in-two-stores

### define_func

    define_func-called-by-a-module

### call

    call-a-trap-answered-and-the-store-runs-on

### BRIDGE_ERR

    BRIDGE_ERR-nothing-above-an-error-written

### BRIDGE_WARN

    BRIDGE_WARN-a-warning-and-what-is-worse

### BRIDGE_INFO

    BRIDGE_INFO-information-and-what-is-worse

### BRIDGE_DEBUG

    BRIDGE_DEBUG-every-line-written

### BRIDGE_LOG_ERR

    BRIDGE_LOG_ERR-written-as-severity-3

### BRIDGE_LOG_WARN

    BRIDGE_LOG_WARN-written-as-severity-4

### BRIDGE_LOG_INFO

    BRIDGE_LOG_INFO-written-as-severity-6

### BRIDGE_LOG_DEBUG

    BRIDGE_LOG_DEBUG-written-as-severity-7

### bridge_list_modules

    bridge_list_modules-fills-no-more-than-asked
    bridge_list_modules-what-is-loaded-and-whether-it-runs

### bridge_value_t

    bridge_value_t-a-value-of-each-kind

### setting.v

    setting.v-seeds-the-field-its-key-names

### artifact.len

    artifact.len-as-many-bytes-compiled

### record.identity

    record.identity-whose-it-is

### record.system

    record.system-which-system-said-it

### record.text

    record.text-what-was-said

### record.len

    record.len-as-long-as-the-text

### record.when

    record.when-the-barrier-it-was-said-after

### record.arg

    record.arg-the-two-numbers

### module_info.identity

    module_info.identity-of-each-that-is-loaded

### module_info.state

    module_info.state-running-and-stopped

### bridge_trigger_t

    bridge_trigger_t-who-triggered-and-how

### bridge_text_t

    bridge_text_t-a-string-a-component-keeps

### bridge_log_record_t

    bridge_log_record_t-written-by-a-module-and-drained

### backend_create

    backend_create-a-setting-refused

### BRIDGE_SPAN_SIZE

    BRIDGE_SPAN_SIZE-what-a-span-takes-of-the-spans

### BRIDGE_SCHEMA_TEXT

    BRIDGE_SCHEMA_TEXT-a-descriptor-of-the-type-s-own-text

### BRIDGE_SCHEMA_STRUCT_TYPE

    BRIDGE_SCHEMA_STRUCT_TYPE-laid-out-as-its-descriptor

### BRIDGE_SCHEMA_STRUCT_DESC

    BRIDGE_SCHEMA_STRUCT_DESC-read-and-agreed-with

### BRIDGE_SCHEMA_ENUM_TYPE

    BRIDGE_SCHEMA_ENUM_TYPE-of-the-integer-stated

### BRIDGE_SCHEMA_ENUM_DESC

    BRIDGE_SCHEMA_ENUM_DESC-read-and-agreed-with

### BRIDGE_SCHEMA_BITMASK_TYPE

    BRIDGE_SCHEMA_BITMASK_TYPE-of-the-integer-stated

### BRIDGE_SCHEMA_BITMASK_DESC

    BRIDGE_SCHEMA_BITMASK_DESC-read-and-agreed-with

### backend_import_t

    backend_import_t-the-two-memories-found

### backend_source_t

    backend_source_t-no-bytes-refused

### module_list_imports

    module_list_imports-the-two-memories-lent

### module_destroy

    module_destroy-a-failed-load-leaves-nothing

### instance_destroy

    instance_destroy-an-unload-gives-its-share-back

### instance_set_work_limit

    instance_set_work_limit-a-spinning-system-ended

### instance_set_timeout

    instance_set_timeout-a-spinning-init-interrupted

### instance_set_memory_limit

    instance_set_memory_limit-growth-refused

### BACKEND_IMPORT_OTHER

    BACKEND_IMPORT_OTHER-a-global-nobody-answers

### BRIDGE_BOOL

    BRIDGE_BOOL-seeds-a-bool

### BRIDGE_FLOAT

    BRIDGE_FLOAT-seeds-a-float

### BRIDGE_RUNNING

    BRIDGE_RUNNING-a-module-that-still-runs

### BRIDGE_STOPPED

    BRIDGE_STOPPED-a-module-at-its-failures

### BRIDGE_HOST_ABI

    BRIDGE_HOST_ABI-stated-and-created

### BRIDGE_LEVEL_MOST

    BRIDGE_LEVEL_MOST-every-line-written

### BRIDGE_STRUCT

    BRIDGE_STRUCT-a-struct-a-field-names

### BRIDGE_ENUM

    BRIDGE_ENUM-an-enum-a-field-names

### BRIDGE_BITMASK

    BRIDGE_BITMASK-a-bitmask-a-field-names

### BRIDGE_TABLE

    BRIDGE_TABLE-walked-by-its-rows

### BRIDGE_READ

    BRIDGE_READ-a-term-that-reads

### BRIDGE_AND

    BRIDGE_AND-only-the-entities-that-carry-both

### BRIDGE_OPTIONAL

    BRIDGE_OPTIONAL-a-term-an-entity-may-lack

### BRIDGE_ON_ENTER

    BRIDGE_ON_ENTER-told-with-the-attach-that-made-it

### BRIDGE_ON_ATTACH

    BRIDGE_ON_ATTACH-told-alone-of-an-attach-again

### BRIDGE_ON_LEAVE

    BRIDGE_ON_LEAVE-told-of-what-was-destroyed

### BRIDGE_ABI

    BRIDGE_ABI-stated-and-loaded

### BRIDGE_NAMESPACE

    BRIDGE_NAMESPACE-where-every-import-is

### BRIDGE_SHARED

    BRIDGE_SHARED-the-memory-two-modules-read

### BRIDGE_PRIVATE

    BRIDGE_PRIVATE-the-memory-of-one-module

### BRIDGE_CARRIED

    BRIDGE_CARRIED-where-a-tag-would-have-a-value

### BRIDGE_ZEROED

    BRIDGE_ZEROED-attached-with-no-value

### BRIDGE_SPAN_BASE

    BRIDGE_SPAN_BASE-where-a-span-keeps-its-columns

### BACKEND_IMPORT_FUNC

    BACKEND_IMPORT_FUNC-an-entry-imported-is-no-memory

### BACKEND_IMPORT_MEMORY

    BACKEND_IMPORT_MEMORY-the-two-memories-lent

### BACKEND_I32

    BACKEND_I32-an-argument-of-32-bits

### BACKEND_I64

    BACKEND_I64-an-argument-of-64-bits

### config.backends

    config.backends-none-refused

## The tree

Each tool that holds the tree, as a case.

### c23

    c23

### style

    style

### deps

    deps

### said

    said

### cover

    cover

### xref

    xref

### licence

    licence

### package

    package

### cla

    cla-the-holder-is-not-asked
    cla-a-name-on-record-passes
    cla-anybody-else-is-asked-and-fails
    cla-the-sentence-by-its-author-is-recorded
    cla-the-sentence-by-another-is-not-taken
    cla-other-words-are-not-taken
