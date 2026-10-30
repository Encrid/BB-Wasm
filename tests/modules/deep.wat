;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (func $down (param $n i32) (result i32)
    (if (result i32) (i32.eqz (local.get $n))
      (then (i32.const 0))
      (else (call $down (i32.sub (local.get $n) (i32.const 1))))))
  (func (export "init") (result i32) (call $down (i32.const 4096)))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
