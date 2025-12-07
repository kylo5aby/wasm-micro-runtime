(module
  ;; Import section - none needed for this test

  ;; Type section - define function signatures
  (type $f64_to_i32 (func (param f64) (result i32)))

  ;; Export section - make functions accessible from host
  (export "test_i32_trunc_f64_s_basic" (func $test_i32_trunc_f64_s_basic))
  (export "test_i32_trunc_f64_s_overflow" (func $test_i32_trunc_f64_s_overflow))

  ;; Code section - function implementations

  ;; Basic truncation function for normal values
  (func $test_i32_trunc_f64_s_basic (param $input f64) (result i32)
    (local.get $input)      ;; Load f64 parameter onto stack
    (i32.trunc_f64_s)       ;; Convert f64 to signed i32 with truncation
    ;; Result i32 is automatically returned from stack
  )

  ;; Overflow testing function - expects values that should trap
  (func $test_i32_trunc_f64_s_overflow (param $input f64) (result i32)
    (local.get $input)      ;; Load f64 parameter onto stack
    (i32.trunc_f64_s)       ;; This should trap for overflow/special values
    ;; If we reach here, no trap occurred (unexpected for overflow test)
  )
)