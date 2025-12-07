(module
  ;; f64.ge test module
  ;; Tests the f64.ge (greater than or equal) opcode with comprehensive test cases
  ;; covering IEEE 754 double-precision floating-point comparison semantics

  ;; f64_ge_test function: Tests f64.ge opcode with two f64 parameters
  ;; Parameters: $a (f64) - first operand, $b (f64) - second operand
  ;; Returns: i32 (1 if $a >= $b, 0 otherwise)
  (func $f64_ge_test (export "f64_ge_test") (param $a f64) (param $b f64) (result i32)
    ;; Load both parameters onto stack and perform f64.ge comparison
    local.get $a      ;; Push first operand ($a) onto stack
    local.get $b      ;; Push second operand ($b) onto stack
    f64.ge            ;; Perform f64.ge comparison: $a >= $b
    ;; Result (i32): 1 if $a >= $b, 0 otherwise
  )
)