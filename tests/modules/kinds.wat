;; Copyright (C) 2026 Enkrid
;;
;; Under the GNU Affero General Public License, version 3 only: see license.
(module
  (memory 1)
  (table $t 2 funcref)
  (type $none (func))
  (data $d "x")
  (elem $e func $all)
  (func $all
    (drop (f64.const 1.5))
    (block $a (block $b (br_table $a $b (i32.const 0))))
    (drop (select (result funcref) (ref.null func) (ref.null func)
      (i32.const 0)))
    (call_indirect (type $none) (i32.const 0))
    (table.init $t $e (i32.const 0) (i32.const 0) (i32.const 1))
    (table.copy $t $t (i32.const 0) (i32.const 1) (i32.const 1))
    (memory.fill (i32.const 0) (i32.const 0) (i32.const 1))
    (memory.init $d (i32.const 0) (i32.const 0) (i32.const 1))
    (data.drop $d)
    (memory.copy (i32.const 1) (i32.const 0) (i32.const 1))
    (drop (memory.size))
    (drop (ref.is_null (ref.null func)))
    (drop (ref.func $all))
    (drop (i32.trunc_sat_f32_s (f32.const 2.5)))
    (drop (i8x16.extract_lane_s 3
      (i8x16.shuffle 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15
        (v128.const i32x4 1 2 3 4) (v128.load32_zero (i32.const 0)))))
    (drop (i8x16.swizzle (v128.load64_zero (i32.const 0))
      (i8x16.eq (v128.const i32x4 0 0 0 0) (v128.const i32x4 0 0 0 0))))
    (drop (v128.load8_lane 0 (i32.const 0) (v128.const i32x4 0 0 0 0))))
  (func (export "init") (result i32) (i32.const 0))
  (func (export "shutdown") (result i32) (i32.const 0))
  (func (export "reload") (result i32) (i32.const 0)))
