;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (memory 1)
  (func (export "init") (result i32)
    (if (result i32) (i32.eq (memory.grow (i32.const 64)) (i32.const -1))
      (then (i32.const 7))
      (else (i32.const 0))))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
