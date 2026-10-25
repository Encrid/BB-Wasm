;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (import "bridge" "ping" (func $ping (param i32 i64) (result i32)))
  (func (export "init") (result i32)
    (call $ping (i32.const 7) (i64.const 9))))
