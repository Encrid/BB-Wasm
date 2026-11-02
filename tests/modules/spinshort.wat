;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (func (export "init") (result i32)
    (local $left i32)
    (local.set $left (i32.const 1000))
    (loop $again
      (local.set $left (i32.sub (local.get $left) (i32.const 1)))
      (br_if $again (local.get $left)))
    (i32.const 0)))
