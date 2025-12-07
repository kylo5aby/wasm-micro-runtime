;; f64.floor Test Module
;;
;; This WebAssembly text format module provides test functions for the f64.floor opcode.
;; The module exports a single test function that applies f64.floor to input values.
;;
;; Test Function:
;; - test_f64_floor: Takes one f64 parameter and returns f64.floor of that value
;;
;; Usage: Load this module in WAMR and call test_f64_floor with various f64 inputs
;; to validate the floor operation behavior across different value ranges and special cases.

(module
  ;; Export the f64.floor test function for external calling
  (export "test_f64_floor" (func $test_f64_floor))

  ;; Test function for f64.floor opcode
  ;; Parameter: $input (f64) - The value to apply floor operation to
  ;; Returns: (f64) - The result of f64.floor($input)
  (func $test_f64_floor (param $input f64) (result f64)
    ;; Load the input parameter onto the stack
    local.get $input

    ;; Apply the f64.floor operation
    ;; This pops the f64 value from stack and pushes floor(value) back
    f64.floor

    ;; Return the result (f64 value on top of stack)
  )
)