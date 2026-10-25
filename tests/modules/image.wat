;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (memory 1)
  (data (i32.const 0) "static")
  (func (export "init") (result i32)
    (i32.sub (i32.load8_u (i32.const 0)) (i32.const 115)))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
