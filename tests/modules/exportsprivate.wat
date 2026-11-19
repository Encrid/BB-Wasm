;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (import "bridge" "private" (memory 1))
  (export "lent" (memory 0))
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
