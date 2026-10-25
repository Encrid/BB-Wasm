;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (func $boom unreachable)
  (start $boom)
  (func (export "init") (result i32) (i32.const 0)))
