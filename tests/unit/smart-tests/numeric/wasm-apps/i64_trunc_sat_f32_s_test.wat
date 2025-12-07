(module
  ;; Test function for i64.trunc_sat_f32_s opcode
  ;; Takes f32 parameter and returns i64 result of saturating signed truncation
  (func (export "test_i64_trunc_sat_f32_s") (param f32) (result i64)
    ;; Load f32 parameter from stack
    local.get 0

    ;; Perform saturating truncation: f32 -> i64 (signed)
    ;; This uses the miscellaneous extension opcode 0xFC 0x04
    i64.trunc_sat_f32_s

    ;; Return i64 result
  )
)