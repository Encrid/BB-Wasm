;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (import "bridge" "private" (memory $private 1))
  (func $set (param $at i32) (param $v i32)
    (if (i32.or
          (i32.gt_u (i32.add (local.get $at) (i32.const 4)) (i32.load $private (i32.const 32)))
          (i32.and (i32.lt_u (local.get $at) (i32.const 128))
            (i32.and (i32.ne (local.get $at) (i32.const 12)) (i32.ne (local.get $at) (i32.const 24)))))
      (then (unreachable)))
    (i32.store $private (local.get $at) (local.get $v)))
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
