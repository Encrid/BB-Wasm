;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (import "bridge" "use_stack" (func $use_stack (result i32)))
  (func $down (result i32)
    (drop (call $use_stack))
    (call $down))
  (func (export "init") (result i32) (call $down)))
