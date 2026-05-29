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

  (func $WidgetDemoApp (export "WidgetDemoApp")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 0
    i32.store offset=0
    local.get $this
    i32.const 50
    i32.store offset=4
    local.get $this
    i32.const 0
    i32.store offset=8
    local.get $this
    i32.const 1024
    i32.store offset=12
    global.get $free_mem
    i32.const 16
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $WidgetDemoApp_build (export "WidgetDemoApp_build")
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
    (local $tmp_ui_13 i32)
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    i32.const 1033
    call $create_text
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    i32.const 1065
    i32.const 1076
    call $set_attribute
    local.get $tmp_ui_1
    i32.const 1081
    i32.const 1087
    call $set_attribute
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1092
    local.get $this
    i32.load offset=0
    call $concat_int
    call $create_text
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1081
    i32.const 1112
    call $set_attribute
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_2
    i32.const 1119
    local.get $this
    i32.const 1145
    call $create_checkbox
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    i32.const 1174
    local.get $this
    i32.load offset=0
    call $set_attribute_int
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_2
    i32.const 1182
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    i32.const 1190
    local.get $this
    i32.load offset=4
    call $concat_int
    call $create_text
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    i32.const 1081
    i32.const 1208
    call $set_attribute
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_5
    local.get $this
    i32.const 1214
    call $create_slider
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    i32.const 1241
    i32.const 0
    call $set_attribute_int
    local.get $tmp_ui_7
    i32.const 1245
    i32.const 100
    call $set_attribute_int
    local.get $tmp_ui_7
    i32.const 1249
    local.get $this
    i32.load offset=4
    call $set_attribute_int
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_5
    i32.const 1182
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    i32.const 1255
    local.get $this
    i32.load offset=8
    call $concat_int
    call $create_text
    local.set $tmp_ui_9
    local.get $tmp_ui_9
    i32.const 1081
    i32.const 1280
    call $set_attribute
    local.get $tmp_ui_9
    call $append_child
    local.get $tmp_ui_8
    i32.const 1288
    local.get $this
    i32.const 1309
    call $create_toggle
    local.set $tmp_ui_10
    local.get $tmp_ui_10
    i32.const 1336
    local.get $this
    i32.load offset=8
    call $set_attribute_int
    local.get $tmp_ui_10
    call $append_child
    local.get $tmp_ui_8
    i32.const 1182
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_11
    local.get $tmp_ui_11
    i32.const 1341
    local.get $this
    i32.load offset=12
    call $concat_int
    call $create_text
    local.set $tmp_ui_12
    local.get $tmp_ui_12
    i32.const 1081
    i32.const 1087
    call $set_attribute
    local.get $tmp_ui_12
    call $append_child
    local.get $tmp_ui_11
    i32.const 1362
    local.get $this
    i32.const 1389
    call $create_dropdown
    local.set $tmp_ui_13
    local.get $tmp_ui_13
    i32.const 1249
    local.get $this
    i32.load offset=12
    call $set_attribute_int
    local.get $tmp_ui_13
    call $append_child
    local.get $tmp_ui_11
    i32.const 1182
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_11
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $WidgetDemoApp_handleCheckbox (export "WidgetDemoApp_handleCheckbox")
    (param $this i32)
    (param $checked i32)
    local.get $this
    local.get $checked
    i32.store offset=0
    local.get $this
    local.get $this
    call $WidgetDemoApp_build
    call $render
  )
  (func $WidgetDemoApp_handleSlider (export "WidgetDemoApp_handleSlider")
    (param $this i32)
    (param $val i32)
    local.get $this
    local.get $val
    i32.store offset=4
    local.get $this
    local.get $this
    call $WidgetDemoApp_build
    call $render
  )
  (func $WidgetDemoApp_handleToggle (export "WidgetDemoApp_handleToggle")
    (param $this i32)
    (param $onState i32)
    local.get $this
    local.get $onState
    i32.store offset=8
    local.get $this
    local.get $this
    call $WidgetDemoApp_build
    call $render
  )
  (func $WidgetDemoApp_handleDropdown (export "WidgetDemoApp_handleDropdown")
    (param $this i32)
    (param $choice i32)
    local.get $this
    local.get $choice
    i32.store offset=12
    local.get $this
    local.get $this
    call $WidgetDemoApp_build
    call $render
  )
  (func $main (export "main")
    (local $app i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    i32.const 1418
    call $println
    call $WidgetDemoApp
    local.set $app
    local.get $app
    local.get $app
    call $WidgetDemoApp_build
    call $render
    i32.const 1462
    call $println
  )
  (data (i32.const 1462) "--- Form widgets loaded. Interactive events ready. ---\00")
  (data (i32.const 1418) "--- Booting Zenith Form Widget Demo App ---\00")
  (data (i32.const 1389) "WidgetDemoApp_handleDropdown\00")
  (data (i32.const 1336) "isOn\00")
  (data (i32.const 1309) "WidgetDemoApp_handleToggle\00")
  (data (i32.const 1280) "magenta\00")
  (data (i32.const 1249) "value\00")
  (data (i32.const 1245) "max\00")
  (data (i32.const 1241) "min\00")
  (data (i32.const 1214) "WidgetDemoApp_handleSlider\00")
  (data (i32.const 1208) "green\00")
  (data (i32.const 1076) "bold\00")
  (data (i32.const 1033) "ZENITH WIDGET VERIFICATION DEMO\00")
  (data (i32.const 1362) "Option 1,Option 2,Option 3\00")
  (data (i32.const 1288) "Toggle Feature Alpha\00")
  (data (i32.const 1065) "fontWeight\00")
  (data (i32.const 1112) "yellow\00")
  (data (i32.const 1255) "3. Toggle Switch State: \00")
  (data (i32.const 1081) "color\00")
  (data (i32.const 1024) "Option 1\00")
  (data (i32.const 1087) "cyan\00")
  (data (i32.const 1145) "WidgetDemoApp_handleCheckbox\00")
  (data (i32.const 1092) "1. Checkbox State: \00")
  (data (i32.const 1119) "Accept Terms & Conditions\00")
  (data (i32.const 1341) "4. Dropdown Chosen: \00")
  (data (i32.const 1174) "checked\00")
  (data (i32.const 1182) "padding\00")
  (data (i32.const 1190) "2. Slider Value: \00")
)
