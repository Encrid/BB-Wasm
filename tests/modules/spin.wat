;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (func (export "init") (result i32)
    (loop $again (br $again))
    (unreachable)))
