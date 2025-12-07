(module
  ;; Test module for f64.promote_f32 stack management validation
  ;; This module demonstrates proper stack usage for f64.promote_f32

  ;; Function that demonstrates minimal correct usage
  ;; @return f64 - Should succeed with proper stack management
  (func $minimal_usage (result f64)
    f32.const 1.0   ;; Push f32 value onto stack
    f64.promote_f32 ;; Promote with proper stack state
  )

  ;; Function that demonstrates edge case stack management
  ;; @return f64 - Should succeed even with complex stack operations
  (func $complex_stack_usage (result f64)
    f32.const 2.5   ;; Push first f32 value
    f32.const 3.5   ;; Push second f32 value
    drop            ;; Remove top f32, leaving 2.5 on stack
    f64.promote_f32 ;; Promote remaining f32 value
  )

  ;; Function that demonstrates correct stack management for comparison
  ;; @return f64 - Should succeed with proper stack management
  (func $correct_usage (result f64)
    f32.const 2.5   ;; Push f32 value onto stack
    f64.promote_f32 ;; Promote with proper stack state
  )

  ;; Export functions for stack management testing
  (export "minimal_usage" (func $minimal_usage))
  (export "complex_stack_usage" (func $complex_stack_usage))
  (export "correct_usage" (func $correct_usage))
)