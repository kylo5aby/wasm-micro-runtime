(module
  ;; i32 arithmetic test function
  (func $i32_add (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.add)
  
  (func $i32_sub (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.sub)
  
  (func $i32_mul (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.mul)
  
  (func $i32_div_s (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.div_s)
  
  ;; i64 arithmetic test function
  (func $i64_add (param $a i64) (param $b i64) (result i64)
    local.get $a
    local.get $b
    i64.add)
  
  ;; f32 arithmetic test function
  (func $f32_add (param $a f32) (param $b f32) (result f32)
    local.get $a
    local.get $b
    f32.add)
  
  ;; f64 arithmetic test function
  (func $f64_add (param $a f64) (param $b f64) (result f64)
    local.get $a
    local.get $b
    f64.add)
  
  ;; Logical operations
  (func $i32_and (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.and)
  
  (func $i32_or (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.or)
  
  ;; Comparison operations
  (func $i32_eq (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.eq)
  
  (func $i32_lt_s (param $a i32) (param $b i32) (result i32)
    local.get $a
    local.get $b
    i32.lt_s)
  
  ;; Constants and locals
  (func $const_test (result i32)
    i32.const 42)
  
  (func $local_test (param $input i32) (result i32)
    (local $temp i32)
    local.get $input
    i32.const 10
    i32.add
    local.set $temp
    local.get $temp)
  
  ;; Type conversion
  (func $i32_wrap_i64 (param $a i64) (result i32)
    local.get $a
    i32.wrap_i64)
  
  (func $f32_convert_i32_s (param $a i32) (result f32)
    local.get $a
    f32.convert_i32_s)
  
  ;; Exports
  (export "i32_add" (func $i32_add))
  (export "i32_sub" (func $i32_sub))
  (export "i32_mul" (func $i32_mul))
  (export "i32_div_s" (func $i32_div_s))
  (export "i64_add" (func $i64_add))
  (export "f32_add" (func $f32_add))
  (export "f64_add" (func $f64_add))
  (export "i32_and" (func $i32_and))
  (export "i32_or" (func $i32_or))
  (export "i32_eq" (func $i32_eq))
  (export "i32_lt_s" (func $i32_lt_s))
  (export "const_test" (func $const_test))
  (export "local_test" (func $local_test))
  (export "i32_wrap_i64" (func $i32_wrap_i64))
  (export "f32_convert_i32_s" (func $f32_convert_i32_s))
)