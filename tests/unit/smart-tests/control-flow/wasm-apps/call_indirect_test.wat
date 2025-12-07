(module
  ;; Function type declarations for different signatures
  (type $type0 (func (param i32) (result i32)))          ;; Type 0: (i32) -> (i32)
  (type $type1 (func (result i32)))                       ;; Type 1: () -> (i32)
  (type $type2 (func (param i32 i32) (result i32)))      ;; Type 2: (i32 i32) -> (i32)
  (type $type3 (func (param i32) (result i32 i32)))      ;; Type 3: (i32) -> (i32 i32)
  (type $type4 (func (param i32 f64) (result i64)))      ;; Type 4: (i32 f64) -> (i64)
  (type $type5 (func (param f64) (result f64)))          ;; Type 5: (f64) -> (f64) - for type mismatch testing

  ;; Function table with 8 slots for testing various scenarios
  (table 8 8 funcref)

  ;; Test functions for different signatures

  ;; Function 0: Simple doubler (i32) -> (i32)
  (func $double_i32 (type $type0)
    local.get 0
    i32.const 2
    i32.mul
  )

  ;; Function 1: Constant returner () -> (i32)
  (func $return_const (type $type1)
    i32.const 123
  )

  ;; Function 2: Adder (i32 i32) -> (i32)
  (func $add_i32 (type $type2)
    local.get 0
    local.get 1
    i32.add
  )

  ;; Function 3: Identity + square (i32) -> (i32 i32)
  (func $identity_and_square (type $type3)
    local.get 0
    local.get 0
    local.get 0
    i32.mul
  )

  ;; Function 4: Complex mixed type (i32 f64) -> (i64)
  (func $mixed_type_func (type $type4)
    local.get 0
    i64.extend_i32_s
    local.get 1
    i64.trunc_f64_s
    i64.add
  )

  ;; Function 5: F64 function for type mismatch testing (f64) -> (f64)
  (func $f64_identity (type $type5)
    local.get 0
  )

  ;; Initialize function table with test functions
  (elem (i32.const 0) $double_i32)           ;; Index 0: doubler
  (elem (i32.const 1) $double_i32)           ;; Index 1: doubler (same signature for boundary test)
  (elem (i32.const 2) $add_i32)              ;; Index 2: adder
  (elem (i32.const 3) $double_i32)           ;; Index 3: another doubler (for boundary test)
  (elem (i32.const 4) $mixed_type_func)      ;; Index 4: complex function
  (elem (i32.const 5) $identity_and_square)  ;; Index 5: multi-return
  (elem (i32.const 6) $return_const)         ;; Index 6: constant (for void test)
  (elem (i32.const 7) $f64_identity)         ;; Index 7: type mismatch function

  ;; Exported test wrapper functions

  ;; Test basic call_indirect with i32 parameter
  (func $test_call_indirect_i32 (export "test_call_indirect_i32") (param i32 i32) (result i32)
    local.get 0          ;; Push function parameter
    local.get 1          ;; Push table index
    call_indirect (type $type0)  ;; Call function through table
  )

  ;; Test call_indirect with void parameters
  (func $test_call_indirect_void (export "test_call_indirect_void") (param i32) (result i32)
    local.get 0          ;; Push table index
    call_indirect (type $type1)  ;; Call () -> (i32) function
  )

  ;; Test call_indirect with multiple parameters
  (func $test_call_indirect_multi (export "test_call_indirect_multi") (param i32 i32 i32) (result i32)
    local.get 0          ;; Push first parameter
    local.get 1          ;; Push second parameter
    local.get 2          ;; Push table index
    call_indirect (type $type2)  ;; Call (i32 i32) -> (i32) function
  )

  ;; Test boundary access with valid indices
  (func $test_call_indirect_boundary (export "test_call_indirect_boundary") (param i32 i32) (result i32)
    local.get 0          ;; Push function parameter
    local.get 1          ;; Push table index
    call_indirect (type $type0)  ;; Call (i32) -> (i32) function
  )

  ;; Test complex function signatures
  (func $test_call_indirect_complex (export "test_call_indirect_complex") (param i32 f64 i32) (result i64)
    local.get 0          ;; Push i32 parameter
    local.get 1          ;; Push f64 parameter
    local.get 2          ;; Push table index
    call_indirect (type $type4)  ;; Call (i32 f64) -> (i64) function
  )

  ;; Test function with multiple return values
  (func $test_call_indirect_multi_return (export "test_call_indirect_multi_return") (param i32 i32) (result i32 i32)
    local.get 0          ;; Push function parameter
    local.get 1          ;; Push table index
    call_indirect (type $type3)  ;; Call (i32) -> (i32 i32) function
  )

  ;; Test type mismatch scenario (will trap when called with wrong signature)
  (func $test_call_indirect_type_mismatch (export "test_call_indirect_type_mismatch") (param i32 i32) (result i32)
    local.get 0          ;; Push i32 parameter
    local.get 1          ;; Push table index (should point to f64 function)
    call_indirect (type $type0)  ;; Try to call as (i32) -> (i32) but function expects (f64) -> (f64)
  )

  ;; Memory for any additional test data (though not used in these tests)
  (memory 1)
)