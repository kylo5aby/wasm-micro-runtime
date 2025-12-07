;; WebAssembly Text Format (WAT) for f64.sub validation testing
;;
;; This module creates a valid WASM module that can be loaded successfully
;; but demonstrates proper WAMR error handling and validation scenarios.
;; Since WASM validation prevents stack underflow at load time, we create
;; a valid module for testing the error handling path in the C++ test.
;;
;; Test Functions:
;; - test_valid_f64_sub: Valid f64.sub operation for baseline testing
;;
;; Note: Actual stack underflow is prevented by WASM validation,
;; so this module tests the error handling path in WAMR when loading
;; potentially problematic modules.

(module
  ;; Export test function for validation testing
  (export "test_valid_f64_sub" (func $test_valid_f64_sub))

  ;; Valid f64.sub operation for baseline testing
  ;;
  ;; Parameters:
  ;;   $a (f64): First operand
  ;;   $b (f64): Second operand
  ;;
  ;; Returns:
  ;;   f64: Result of a - b
  ;;
  ;; This function provides a valid f64.sub operation that can be used
  ;; to test the error handling path in the C++ test when a module loads
  ;; successfully but may be used to test edge cases.
  (func $test_valid_f64_sub (param $a f64) (param $b f64) (result f64)
    ;; Load parameters onto stack
    local.get $a             ;; Stack: [a]
    local.get $b             ;; Stack: [a, b]

    ;; Execute valid f64.sub operation
    f64.sub                  ;; Stack: [a - b]

    ;; Return result
  )

  ;; Memory section (minimal)
  (memory 1)
)