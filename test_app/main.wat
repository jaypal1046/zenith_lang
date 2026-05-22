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
  (import "env" "set_attribute" (func $set_attribute (param i32 i32 i32)))
  (import "env" "set_attribute_int" (func $set_attribute_int (param i32 i32 i32)))
  (memory (export "memory") 1)
  (global $free_mem (mut i32) (i32.const 8192))

  (func $MyApp (export "MyApp")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 1024
    i32.store offset=0
    local.get $this
    i32.const 0
    i32.store offset=4
    local.get $this
    i32.const 1050
    i32.store offset=8
    global.get $free_mem
    i32.const 12
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $MyApp_build (export "MyApp_build")
    (param $this i32)
    (result i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    (local $tmp_ui_3 i32)
    (local $tmp_ui_4 i32)
    (local $tmp_ui_5 i32)
    (local $tmp_ui_6 i32)
    (local $tmp_ui_7 i32)
    (local $tmp_ui_8 i32)
    (local $tmp_ui_9 i32)
    (local $tmp_ui_10 i32)
    (local $tmp_ui_11 i32)
    (local $tmp_ui_12 i32)
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    call $create_container
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    local.get $this
    i32.load offset=0
    call $create_text
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1067
    i32.const 1078
    call $set_attribute
    local.get $tmp_ui_2
    i32.const 1083
    i32.const 1089
    call $set_attribute
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_1
    i32.const 1094
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1102
    call $create_text
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    i32.const 1067
    i32.const 1078
    call $set_attribute
    local.get $tmp_ui_4
    i32.const 1083
    i32.const 1123
    call $set_attribute
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_3
    call $create_row
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    i32.const 1130
    local.get $this
    i32.load offset=4
    call $concat_int
    call $create_text
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    i32.const 1083
    i32.const 1138
    call $set_attribute
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_5
    i32.const 1144
    local.get $this
    i32.const 1154
    call $create_button
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_3
    i32.const 1094
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    i32.const 1176
    call $create_text
    local.set $tmp_ui_9
    local.get $tmp_ui_9
    i32.const 1067
    i32.const 1078
    call $set_attribute
    local.get $tmp_ui_9
    i32.const 1083
    i32.const 1206
    call $set_attribute
    local.get $tmp_ui_9
    call $append_child
    local.get $tmp_ui_8
    call $create_row
    local.set $tmp_ui_10
    local.get $tmp_ui_10
    i32.const 1214
    local.get $this
    i32.load offset=8
    call $concat_int
    call $create_text
    local.set $tmp_ui_11
    local.get $tmp_ui_11
    call $append_child
    local.get $tmp_ui_10
    i32.const 1228
    local.get $this
    i32.const 1239
    call $create_button
    local.set $tmp_ui_12
    local.get $tmp_ui_12
    call $append_child
    local.get $tmp_ui_10
    call $append_child
    local.get $tmp_ui_8
    i32.const 1094
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $MyApp_handleIncrement (export "MyApp_handleIncrement")
    (param $this i32)
    local.get $this
    local.get $this
    i32.load offset=4
    i32.const 1
    i32.add
    i32.store offset=4
    local.get $this
    local.get $this
    call $MyApp_build
    call $render
  )
  (func $MyApp_handleFetch (export "MyApp_handleFetch")
    (param $this i32)
    (local $res i32)
    (local $tmp_ui_0 i32)
    i32.const 1257
    call $httpGet
    local.set $res
    local.get $this
    local.get $res
    i32.store offset=8
    local.get $this
    local.get $this
    call $MyApp_build
    call $render
  )
  (func $main (export "main")
    (local $app i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    i32.const 1302
    call $println
    call $MyApp
    local.set $app
    local.get $app
    local.get $app
    call $MyApp_build
    call $render
    i32.const 1337
    call $println
  )
  (data (i32.const 1257) "https://jsonplaceholder.typicode.com/todos/1\00")
  (data (i32.const 1337) "Zenith Application Shutdown.\00")
  (data (i32.const 1239) "MyApp_handleFetch\00")
  (data (i32.const 1206) "magenta\00")
  (data (i32.const 1024) "Zenith Cross-Platform App\00")
  (data (i32.const 1302) "Initializing Zenith Application...\00")
  (data (i32.const 1078) "bold\00")
  (data (i32.const 1050) "No data fetched.\00")
  (data (i32.const 1067) "fontWeight\00")
  (data (i32.const 1214) "API Payload: \00")
  (data (i32.const 1123) "yellow\00")
  (data (i32.const 1083) "color\00")
  (data (i32.const 1176) "REST Network API Verification\00")
  (data (i32.const 1089) "cyan\00")
  (data (i32.const 1154) "MyApp_handleIncrement\00")
  (data (i32.const 1228) "Fetch Data\00")
  (data (i32.const 1138) "green\00")
  (data (i32.const 1130) "Value: \00")
  (data (i32.const 1094) "padding\00")
  (data (i32.const 1102) "Click Counter Sample\00")
  (data (i32.const 1144) "Increment\00")
)
