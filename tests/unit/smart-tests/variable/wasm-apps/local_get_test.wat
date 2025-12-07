;; WebAssembly Text Format (.wat) for local.get Opcode Testing
;; Comprehensive test module for local variable access operation
;;
;; This module provides multiple test functions to validate local.get behavior
;; across various data types and scenarios.

(module
  ;; Test function for i32 local.get opcode
  ;; Takes i32 parameter, stores in local, then retrieves and returns it
  (func (export "test_get_i32_local") (param $input i32) (result i32)
    (local $local_var i32)
    ;; Store parameter in local variable
    local.get $input
    local.set $local_var
    ;; Retrieve local variable value using local.get
    local.get $local_var
    ;; Return result (automatically consumed from stack)
  )

  ;; Test function for i64 local.get opcode
  ;; Takes i64 parameter, stores in local, then retrieves and returns it
  (func (export "test_get_i64_local") (param $input i64) (result i64)
    (local $local_var i64)
    ;; Store parameter in local variable
    local.get $input
    local.set $local_var
    ;; Retrieve local variable value using local.get
    local.get $local_var
    ;; Return result
  )

  ;; Test function for f32 local.get opcode
  ;; Takes f32 parameter, stores in local, then retrieves and returns it
  (func (export "test_get_f32_local") (param $input f32) (result f32)
    (local $local_var f32)
    ;; Store parameter in local variable
    local.get $input
    local.set $local_var
    ;; Retrieve local variable value using local.get
    local.get $local_var
    ;; Return result
  )

  ;; Test function for f64 local.get opcode
  ;; Takes f64 parameter, stores in local, then retrieves and returns it
  (func (export "test_get_f64_local") (param $input f64) (result f64)
    (local $local_var f64)
    ;; Store parameter in local variable
    local.get $input
    local.set $local_var
    ;; Retrieve local variable value using local.get
    local.get $local_var
    ;; Return result
  )

  ;; Test function for multiple local variables with different indices
  (func (export "test_multiple_locals") (param $a i32) (param $b i32) (result i32)
    (local $first i32)
    (local $second i32)
    (local $third i32)

    ;; Set multiple local variables
    local.get $a
    local.set $first
    local.get $b
    local.set $second

    ;; Use local.get with different indices
    local.get $first    ;; Index 2
    local.get $second   ;; Index 3
    i32.add
    local.set $third    ;; Index 4

    ;; Return the third local
    local.get $third
  )

  ;; Test function for uninitialized local variable access
  (func (export "test_uninitialized_local") (result i32)
    (local $uninit i32)
    ;; Access uninitialized local (should return default value 0)
    local.get $uninit
  )

  ;; Test function for parameter access using local.get
  (func (export "test_parameter_access") (param $param1 i32) (param $param2 f32) (result i32)
    ;; Access parameters using local.get (parameters are local variables 0, 1, ...)
    local.get $param1   ;; Access parameter 0
    ;; Ignore param2 for this test, just return param1
  )
)