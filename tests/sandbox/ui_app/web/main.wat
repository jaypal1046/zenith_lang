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
  (import "env" "create_textfield" (func $create_textfield (param i32 i32 i32) (result i32)))
  (import "env" "set_attribute" (func $set_attribute (param i32 i32 i32)))
  (import "env" "set_attribute_int" (func $set_attribute_int (param i32 i32 i32)))
  (memory (export "memory") 1)
  (global $free_mem (mut i32) (i32.const 8192))

  (func $ShowcaseSite (export "ShowcaseSite")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 1024
    i32.store offset=0
    local.get $this
    i32.const 1050
    i32.store offset=4
    local.get $this
    i32.const 1092
    i32.store offset=8
    local.get $this
    i32.const 0
    i32.store offset=12
    global.get $free_mem
    i32.const 16
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $ShowcaseSite_build (export "ShowcaseSite_build")
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
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    local.get $this
    i32.load offset=0
    call $create_text
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    local.get $this
    i32.load offset=8
    call $create_text
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_0
    call $create_row
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1166
    call $create_text
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_3
    local.get $this
    i32.load offset=4
    call $create_text
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_0
    call $create_row
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    i32.const 1175
    local.get $this
    i32.load offset=12
    call $concat_int
    call $create_text
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_6
    i32.const 1190
    local.get $this
    i32.const 1207
    call $create_button
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $ShowcaseSite_increment (export "ShowcaseSite_increment")
    (param $this i32)
    local.get $this
    local.get $this
    i32.load offset=12
    i32.const 1
    i32.add
    i32.store offset=12
    local.get $this
    local.get $this
    call $ShowcaseSite_build
    call $render
  )
  (func $main (export "main")
    (local $site i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    i32.const 1230
    call $println
    call $ShowcaseSite
    local.set $site
    local.get $site
    local.get $site
    call $ShowcaseSite_build
    call $render
    i32.const 1262
    call $println
  )
  (data (i32.const 1262) "--- UI DOM Rendered Successfully from WASM! ---\00")
  (data (i32.const 1230) "--- Booting WASM Zenith App ---\00")
  (data (i32.const 1207) "ShowcaseSite_increment\00")
  (data (i32.const 1190) "Increment Clicks\00")
  (data (i32.const 1175) "Total Clicks: \00")
  (data (i32.const 1092) "No Javascript VMs. No Garbage Collector Pauses. Pure Systems Performance.\00")
  (data (i32.const 1166) "Status: \00")
  (data (i32.const 1050) "Running purely inside the browser sandbox\00")
  (data (i32.const 1024) "ZENITH NATIVE WASM ENGINE\00")
)
