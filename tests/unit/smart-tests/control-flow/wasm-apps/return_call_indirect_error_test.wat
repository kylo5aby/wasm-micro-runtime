(module
  ;; Type definitions for function signatures
  (type $t0 (func (param i32 i32) (result i32)))     ;; Binary operation: (i32, i32) -> i32
  (type $t1 (func (param i32) (result i32)))         ;; Unary operation: i32 -> i32
  (type $t2 (func (result i32)))                     ;; Constant function: () -> i32
  (type $t3 (func (param i32 i32 i32) (result i32))) ;; Triple operation: (i32, i32, i32) -> i32
  (type $wrong_sig (func (param f32) (result f32)))   ;; Wrong signature for testing type mismatch

  ;; Function table with some entries initialized and some null
  ;; Indices 0-4 are initialized, indices 5-19 are null/uninitialized
  (table 20 funcref)
  (elem (i32.const 0) $add_func $mul_func $const42_func $unary_func $triple_func)

  ;; Addition function: adds two i32 values
  (func $add_func (type $t0)
    local.get 0
    local.get 1
    i32.add
  )

  ;; Multiplication function: multiplies two i32 values
  (func $mul_func (type $t0)
    local.get 0
    local.get 1
    i32.mul
  )

  ;; Constant function returning 42
  (func $const42_func (type $t2)
    i32.const 42
  )

  ;; Unary function: returns input * 2
  (func $unary_func (type $t1)
    local.get 0
    i32.const 2
    i32.mul
  )

  ;; Triple parameter function: computes a * b * c
  (func $triple_func (type $t3)
    local.get 0
    local.get 1
    i32.mul
    local.get 2
    i32.mul
  )

  ;; Test functions for error conditions

  ;; Test invalid table index (out of bounds)
  (func $test_invalid_index (export "test_invalid_index") (type $t3) (param $a i32) (param $b i32) (param $invalid_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $invalid_idx  ;; This should be out of bounds (e.g., 100)
    return_call_indirect (type $t0)
  )

  ;; Test null table entry access
  (func $test_null_entry (export "test_null_entry") (type $t3) (param $a i32) (param $b i32) (param $null_idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $null_idx  ;; This should point to null entry (e.g., index 10)
    return_call_indirect (type $t0)
  )

  ;; Test type mismatch (calling unary function with binary signature)
  ;; This will cause a runtime type check failure
  (func $test_type_mismatch (export "test_type_mismatch") (type $t3) (param $a i32) (param $b i32) (param $idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $idx  ;; Index 3 points to unary function but we're using binary signature
    return_call_indirect (type $t0)  ;; Binary signature (i32,i32)->i32 but function is unary
  )

  ;; Test calling triple parameter function with binary signature (parameter count mismatch)
  (func $test_signature_mismatch (export "test_signature_mismatch") (type $t3) (param $a i32) (param $b i32) (param $idx i32) (result i32)
    local.get $a
    local.get $b
    local.get $idx  ;; Index 4 points to triple function but we're using binary signature
    return_call_indirect (type $t0)  ;; Binary signature but function expects 3 params
  )
)