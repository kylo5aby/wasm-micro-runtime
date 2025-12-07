(module
  ;; Simple function that should compile but may have validation differences
  ;; We'll test the invalid index scenario in the C++ test with manual module creation

  (func (export "test_valid_local_set") (param $input i32) (result i32)
    (local $local_var i32)
    local.get $input
    local.set $local_var
    local.get $local_var
  )
)