;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (func (export "init") (result i32)
    (local $left i64)
    (local.set $left (i64.const 9000000000))
    (loop $again
      (local.set $left (i64.sub (local.get $left) (i64.const 1)))
      (br_if $again (i64.ne (local.get $left) (i64.const 0))))
    (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
