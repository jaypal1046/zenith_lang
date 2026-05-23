(module
  (import "env" "print" (func $print (param i32)))
  (import "env" "println" (func $println (param i32)))
  (import "env" "create_column" (func $create_column (result i32)))
  (import "env" "create_row" (func $create_row (result i32)))
  (import "env" "create_text" (func $create_text (param i32) (result i32)))
  (import "env" "append_child" (func $append_child (param i32 i32)))
  (import "env" "render" (func $render (param i32 i32)))
  (import "env" "push" (func $push (param i32)))
  (import "env" "summarizeDocument" (func $summarizeDocument (param i32) (result i32)))
  (import "env" "concat_int" (func $concat_int (param i32 i32) (result i32)))
  (import "env" "concat_float" (func $concat_float (param i32 f64) (result i32)))
  (import "env" "create_button" (func $create_button (param i32 i32 i32) (result i32)))
  (import "env" "httpGet" (func $httpGet (param i32) (result i32)))
  (import "env" "httpPost" (func $httpPost (param i32 i32) (result i32)))
  (import "env" "create_image" (func $create_image (param i32) (result i32)))
  (import "env" "create_video" (func $create_video (param i32) (result i32)))
  (import "env" "create_scrolling" (func $create_scrolling (result i32)))
  (import "env" "create_card" (func $create_card (result i32)))
  (import "env" "create_container" (func $create_container (result i32)))
  (import "env" "create_checkbox" (func $create_checkbox (param i32 i32 i32) (result i32)))
  (import "env" "create_slider" (func $create_slider (param i32 i32) (result i32)))
  (import "env" "create_toggle" (func $create_toggle (param i32 i32 i32) (result i32)))
  (import "env" "create_dropdown" (func $create_dropdown (param i32 i32 i32) (result i32)))
  (import "env" "set_attribute" (func $set_attribute (param i32 i32 i32)))
  (import "env" "set_attribute_int" (func $set_attribute_int (param i32 i32 i32)))
  (memory (export "memory") 1)
  (global $free_mem (mut i32) (i32.const 8192))

  (func $getVerificationStatus (export "getVerificationStatus")
    (result i32)
    i32.const 1024
    return
  )
  (func $CounterApp (export "CounterApp")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 0
    i32.store offset=0
    local.get $this
    i32.const 1060
    i32.store offset=4
    global.get $free_mem
    i32.const 8
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $CounterApp_build (export "CounterApp_build")
    (param $this i32)
    (result i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    local.get $this
    i32.load offset=4
    local.get $this
    i32.load offset=0
    i32.add
    call $create_text
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $CounterApp_increment (export "CounterApp_increment")
    (param $this i32)
    local.get $this
    local.get $this
    i32.load offset=0
    i32.const 1
    i32.add
    i32.store offset=0
    local.get $this
    local.get $this
    call $CounterApp_build
    call $render
  )
  (func $Circle (export "Circle")
    (param $radius f64)
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    local.get $radius
    f64.store offset=0
    global.get $free_mem
    i32.const 8
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $Circle_getArea (export "Circle_getArea")
    (param $this i32)
    (result f64)
    f64.const 3.14159
    local.get $this
    f64.load offset=0
    local.get $this
    f64.load offset=0
    f64.mul
    f64.mul
    return
  )
  (func $Database (export "Database")
    (param $url i32)
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    local.get $url
    i32.store offset=0
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $Database_summarizeDocument (export "Database_summarizeDocument")
    (param $this i32)
    (param $text i32)
    (result i32)
    local.get $text
    call $summarizeDocument
    return
  )
  (func $ChatScreen (export "ChatScreen")
    (result i32)
    (local $active_users i32)
    (local $scores i32)
    (local $active i32)
    (local $retry i32)
    (local $inf_local_str i32)
    (local $ver_status i32)
    (local $full_status i32)
    (local $db i32)
    (local $response i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    (local $tmp_ui_3 i32)
    (local $tmp_ui_4 i32)
    (local $tmp_ui_5 i32)
    (local $tmp_ui_6 i32)
    i32.const 1076
    drop
    i32.const 1080
    drop
    i32.const 0
    local.set $active_users
    i32.const 1076
    drop
    i32.const 100
    drop
    i32.const 1080
    drop
    i32.const 95
    drop
    i32.const 0
    local.set $scores
    i32.const 1
    local.set $active
    i32.const 0
    local.set $retry
    i32.const 1085
    local.set $inf_local_str
    call $getVerificationStatus
    local.set $ver_status
    local.get $ver_status
    local.get $inf_local_str
    call $concat_int
    local.set $full_status
    local.get $active_users
    i32.const 1110
    call $push
    drop
    local.get $active
    i32.const 1
    i32.eq
    (if
      (then
        (block $exit_loop
          (loop $start_loop
            local.get $retry
            i32.const 3
            i32.lt_s
            i32.eqz
            br_if $exit_loop
            local.get $retry
            i32.const 1
            i32.add
            local.set $retry
            br $start_loop
          )
        )
      )
    )
    i32.const 1114
    call $Database
    local.set $db
    local.get $db
    i32.const 1137
    call $Database_summarizeDocument
    local.set $response
    call $create_column
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1165
    call $create_text
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1177
    i32.const 1188
    call $set_attribute
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_2
    call $create_row
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    local.get $response
    call $create_text
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_4
    local.get $full_status
    call $create_text
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_2
    return
  )
  (func $main (export "main")
    (local $app i32)
    (local $counter i32)
    (local $my_shape i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    (local $tmp_ui_3 i32)
    (local $tmp_ui_4 i32)
    (local $tmp_ui_5 i32)
    (local $tmp_ui_6 i32)
    (local $tmp_ui_7 i32)
    i32.const 1193
    call $println
    call $ChatScreen
    local.set $app
    local.get $app
    local.get $app
    call $render
    i32.const 1220
    call $println
    call $CounterApp
    local.set $counter
    local.get $counter
    local.get $counter
    call $CounterApp_build
    call $render
    local.get $counter
    call $CounterApp_increment
    local.get $counter
    call $CounterApp_increment
    i32.const 1259
    call $println
    f64.const 10.0
    call $Circle
    local.set $my_shape
    i32.const 1300
    local.get $my_shape
    call $Circle_getArea
    call $concat_float
    call $println
    i32.const 1314
    call $println
  )
  (data (i32.const 1314) "--- Zenith App Shutdown ---\00")
  (data (i32.const 1259) "\n--- Testing Interface Polymorphism ---\00")
  (data (i32.const 1024) " [Status: Verified (Custom import)]\00")
  (data (i32.const 1076) "Jay\00")
  (data (i32.const 1193) "--- Booting Zenith App ---\00")
  (data (i32.const 1060) "Counter Value: \00")
  (data (i32.const 1080) "Alex\00")
  (data (i32.const 1085) " (Type inferred locally)\00")
  (data (i32.const 1110) "Sam\00")
  (data (i32.const 1137) "This is a document payload.\00")
  (data (i32.const 1114) "http://localhost:11434\00")
  (data (i32.const 1300) "Circle Area: \00")
  (data (i32.const 1165) "AI Summary:\00")
  (data (i32.const 1177) "fontWeight\00")
  (data (i32.const 1188) "bold\00")
  (data (i32.const 1220) "\n--- Testing Counter App setState ---\00")
)
