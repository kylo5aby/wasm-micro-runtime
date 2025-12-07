(module
  ;; Floating point minimum operations
  (func (export "test_f32_min") (param f32 f32) (result f32)
    local.get 0
    local.get 1
    f32.min)
  
  (func (export "test_f32_min_nan") (param f32 f32) (result f32)
    local.get 0
    local.get 1
    f32.min)
  
  ;; Floating point maximum operations
  (func (export "test_f32_max") (param f32 f32) (result f32)
    local.get 0
    local.get 1
    f32.max)
  
  (func (export "test_f32_max_nan") (param f32 f32) (result f32)
    local.get 0
    local.get 1
    f32.max)
  
  ;; Double precision minimum operations
  (func (export "test_f64_min") (param f64 f64) (result f64)
    local.get 0
    local.get 1
    f64.min)
  
  (func (export "test_f64_min_nan") (param f64 f64) (result f64)
    local.get 0
    local.get 1
    f64.min)
  
  ;; Double precision maximum operations
  (func (export "test_f64_max") (param f64 f64) (result f64)
    local.get 0
    local.get 1
    f64.max)
  
  (func (export "test_f64_max_nan") (param f64 f64) (result f64)
    local.get 0
    local.get 1
    f64.max)
  
  ;; Copysign operations
  (func (export "test_f32_copysign") (param f32 f32) (result f32)
    local.get 0
    local.get 1
    f32.copysign)
  
  (func (export "test_f64_copysign") (param f64 f64) (result f64)
    local.get 0
    local.get 1
    f64.copysign)
  
  ;; Special value testing functions
  (func (export "test_f32_special_values") (result f32)
    f32.const nan
    f32.const inf
    f32.min)
  
  (func (export "test_f64_special_values") (result f64)
    f64.const nan
    f64.const inf
    f64.min)
  
  ;; Edge case testing functions
  (func (export "test_f32_zero_handling") (result f32)
    f32.const 0.0
    f32.const -0.0
    f32.min)
  
  (func (export "test_f64_zero_handling") (result f64)
    f64.const 0.0
    f64.const -0.0
    f64.min)
)