;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (table $t 1 funcref)
  (func (export "init") (result i32)
    (if (result i32) (i32.eq (table.grow $t (ref.null func) (i32.const 9)) (i32.const -1))
      (then (i32.const 7))
      (else (i32.const 0))))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
