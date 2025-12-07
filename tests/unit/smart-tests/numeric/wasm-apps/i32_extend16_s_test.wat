(module
  ;; Test module for i32.extend16_s opcode validation
  ;; Provides a single exported function that applies i32.extend16_s to input parameter

  ;; Export the test function for C++ test harness
  (func $i32_extend16_s_test (param $input i32) (result i32)
    ;; Load the input parameter onto the stack
    local.get $input

    ;; Apply i32.extend16_s instruction
    ;; This sign-extends the lower 16 bits to full 32-bit signed integer
    i32.extend16_s
  )

  ;; Make the test function available to the test harness
  (export "i32_extend16_s_test" (func $i32_extend16_s_test))
)