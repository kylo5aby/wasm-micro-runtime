(module
  ;; 0-page initial memory (0KB = 0 pages) with max 10 pages
  (memory (export "memory") 0 10)

  ;; Function to get current memory size
  (func (export "get_memory_size") (result i32)
    memory.size
  )

  ;; Function to grow memory by specified pages and return new size
  (func (export "grow_and_get_size") (param $pages i32) (result i32)
    (local $old_size i32)
    (local.set $old_size (memory.size))
    (drop (memory.grow (local.get $pages)))
    memory.size
  )
)