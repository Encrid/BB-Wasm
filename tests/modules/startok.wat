;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (memory 1)
  (func $mark (i32.store (i32.const 0) (i32.const 1)))
  (start $mark)
  (func (export "init") (result i32) (i32.sub (i32.const 1) (i32.load (i32.const 0))))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
