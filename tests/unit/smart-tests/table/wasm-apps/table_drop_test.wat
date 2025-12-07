(module
  (table 20 funcref)

  ;; Functions must be declared before element segments
  (func $func_0 (result i32) (i32.const 100))
  (func $func_1 (result i32) (i32.const 200))
  (func $func_2 (result i32) (i32.const 300))

  ;; Passive element segments (funcref mode without table declaration)
  (elem funcref (ref.func $func_0) (ref.func $func_1) (ref.func $func_2))
  (elem funcref (ref.func $func_0) (ref.func $func_1) (ref.func $func_2))
  (elem funcref (ref.func $func_0) (ref.func $func_1) (ref.func $func_2))

  (func $test_table_drop (param $elem_index i32) (result i32)
    (if (i32.eq (local.get $elem_index) (i32.const 0))
      (then (elem.drop 0))
      (else
        (if (i32.eq (local.get $elem_index) (i32.const 1))
          (then (elem.drop 1))
          (else (elem.drop 2)))))
    (i32.const 1))

  (func $test_table_init (param $elem_index i32) (param $dest i32) (param $src i32) (param $len i32) (result i32)
    (if (i32.eq (local.get $elem_index) (i32.const 0))
      (then
        (local.get $dest)
        (local.get $src)
        (local.get $len)
        (table.init 0))
      (else
        (if (i32.eq (local.get $elem_index) (i32.const 1))
          (then
            (local.get $dest)
            (local.get $src)
            (local.get $len)
            (table.init 1))
          (else
            (local.get $dest)
            (local.get $src)
            (local.get $len)
            (table.init 2)))))
    (i32.const 1))

  (func $test_invalid_table_drop (param $elem_index i32) (result i32)
    (elem.drop 0)
    (i32.const 1))

  (func $verify_initial_state (result i32)
    (i32.const 1))

  (export "test_table_drop" (func $test_table_drop))
  (export "test_table_init" (func $test_table_init))
  (export "test_invalid_table_drop" (func $test_invalid_table_drop))
  (export "verify_initial_state" (func $verify_initial_state)))