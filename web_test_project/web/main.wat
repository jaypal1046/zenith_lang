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
  (import "env" "triggerBrowserAlert" (func $triggerBrowserAlert (param i32)))
  (import "env" "getBrowserTime" (func $getBrowserTime (result i32)))
  (memory (export "memory") 1)
  (global $free_mem (mut i32) (i32.const 8192))

  (func $AppCommon (export "AppCommon")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppCommon_getCommonMessage (export "AppCommon_getCommonMessage")
    (param $this i32)
    (result i32)
    i32.const 1024
    return
  )
  (func $AppDesktop (export "AppDesktop")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppDesktop_getPlatformName (export "AppDesktop_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1042
    return
  )
  (func $AppAndroid (export "AppAndroid")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppAndroid_getPlatformName (export "AppAndroid_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1069
    return
  )
  (func $AppIos (export "AppIos")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppIos_getPlatformName (export "AppIos_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1084
    return
  )
  (func $AppWeb (export "AppWeb")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppWeb_getPlatformName (export "AppWeb_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1095
    return
  )
  (func $AppLinux (export "AppLinux")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppLinux_getPlatformName (export "AppLinux_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1111
    return
  )
  (func $AppWindows (export "AppWindows")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppWindows_getPlatformName (export "AppWindows_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1136
    return
  )
  (func $AppMac (export "AppMac")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $AppMac_getPlatformName (export "AppMac_getPlatformName")
    (param $this i32)
    (result i32)
    i32.const 1163
    return
  )
  (func $MyApp (export "MyApp")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 1188
    i32.store offset=0
    local.get $this
    i32.const 1208
    i32.store offset=4
    local.get $this
    i32.const 0
    i32.store offset=8
    local.get $this
    i32.const 1
    i32.store offset=12
    local.get $this
    i32.const 24
    i32.store offset=16
    local.get $this
    i32.const 1218
    i32.store offset=20
    local.get $this
    i32.const 1235
    i32.store offset=24
    local.get $this
    call $AppCommon
    i32.store offset=28
    local.get $this
    call $AppAndroid
    i32.store offset=32
    local.get $this
    call $AppIos
    i32.store offset=36
    local.get $this
    call $AppWeb
    i32.store offset=40
    global.get $free_mem
    i32.const 44
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
    (local $tmp_ui_13 i32)
    (local $tmp_ui_14 i32)
    (local $tmp_ui_15 i32)
    (local $tmp_ui_16 i32)
    (local $tmp_ui_17 i32)
    (local $tmp_ui_18 i32)
    (local $tmp_ui_19 i32)
    (local $tmp_ui_20 i32)
    (local $tmp_ui_21 i32)
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    call $create_container
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    i32.const 1263
    local.get $this
    i32.load offset=0
    call $concat_int
    call $create_text
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1275
    i32.const 1286
    call $set_attribute
    local.get $tmp_ui_2
    i32.const 1291
    i32.const 1297
    call $set_attribute
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_1
    i32.const 1302
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1310
    local.get $this
    i32.load offset=4
    i32.const 1318
    i32.add
    call $concat_int
    call $create_text
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    i32.const 1275
    i32.const 1286
    call $set_attribute
    local.get $tmp_ui_4
    i32.const 1291
    i32.const 1320
    call $set_attribute
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_3
    i32.const 1326
    local.get $this
    i32.load offset=16
    call $concat_int
    call $create_text
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_3
    local.get $this
    i32.const 1349
    call $create_slider
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    i32.const 1374
    i32.const 12
    call $set_attribute_int
    local.get $tmp_ui_6
    i32.const 1378
    i32.const 48
    call $set_attribute_int
    local.get $tmp_ui_6
    i32.const 1382
    local.get $this
    i32.load offset=16
    call $set_attribute_int
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_3
    call $create_row
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    i32.const 1388
    call $create_text
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_7
    i32.const 1400
    local.get $this
    i32.const 1416
    call $create_textfield
    local.set $tmp_ui_9
    local.get $tmp_ui_9
    call $append_child
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_3
    i32.const 1302
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_10
    local.get $tmp_ui_10
    i32.const 1439
    call $create_text
    local.set $tmp_ui_11
    local.get $tmp_ui_11
    i32.const 1275
    i32.const 1286
    call $set_attribute
    local.get $tmp_ui_11
    i32.const 1291
    i32.const 1297
    call $set_attribute
    local.get $tmp_ui_11
    call $append_child
    local.get $tmp_ui_10
    i32.const 1467
    local.get $this
    i32.load offset=24
    call $concat_int
    call $create_text
    local.set $tmp_ui_12
    local.get $tmp_ui_12
    i32.const 1291
    i32.const 1491
    call $set_attribute
    local.get $tmp_ui_12
    call $append_child
    local.get $tmp_ui_10
    call $create_row
    local.set $tmp_ui_13
    local.get $tmp_ui_13
    i32.const 1499
    local.get $this
    i32.const 1517
    call $create_button
    local.set $tmp_ui_14
    local.get $tmp_ui_14
    call $append_child
    local.get $tmp_ui_13
    i32.const 1538
    local.get $this
    i32.const 1563
    call $create_button
    local.set $tmp_ui_15
    local.get $tmp_ui_15
    call $append_child
    local.get $tmp_ui_13
    call $append_child
    local.get $tmp_ui_10
    i32.const 1302
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_10
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_16
    local.get $tmp_ui_16
    i32.const 1583
    call $create_text
    local.set $tmp_ui_17
    local.get $tmp_ui_17
    i32.const 1275
    i32.const 1286
    call $set_attribute
    local.get $tmp_ui_17
    i32.const 1291
    i32.const 1604
    call $set_attribute
    local.get $tmp_ui_17
    call $append_child
    local.get $tmp_ui_16
    call $create_row
    local.set $tmp_ui_18
    local.get $tmp_ui_18
    i32.const 1611
    local.get $this
    i32.load offset=8
    call $concat_int
    call $create_text
    local.set $tmp_ui_19
    local.get $tmp_ui_19
    i32.const 1291
    i32.const 1320
    call $set_attribute
    local.get $tmp_ui_19
    call $append_child
    local.get $tmp_ui_18
    i32.const 1620
    local.get $this
    i32.const 1630
    call $create_button
    local.set $tmp_ui_20
    local.get $tmp_ui_20
    call $append_child
    local.get $tmp_ui_18
    call $append_child
    local.get $tmp_ui_16
    i32.const 1652
    local.get $this
    i32.const 1681
    call $create_checkbox
    local.set $tmp_ui_21
    local.get $tmp_ui_21
    i32.const 1705
    local.get $this
    i32.load offset=12
    call $set_attribute_int
    local.get $tmp_ui_21
    call $append_child
    local.get $tmp_ui_16
    i32.const 1302
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_16
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $MyApp_handleNameChange (export "MyApp_handleNameChange")
    (param $this i32)
    (param $name i32)
    local.get $this
    local.get $name
    i32.store offset=4
    local.get $this
    local.get $this
    call $MyApp_build
    call $render
  )
  (func $MyApp_handleSliderChange (export "MyApp_handleSliderChange")
    (param $this i32)
    (param $size i32)
    local.get $this
    local.get $size
    i32.store offset=16
    local.get $this
    local.get $this
    call $MyApp_build
    call $render
  )
  (func $MyApp_handleIncrement (export "MyApp_handleIncrement")
    (param $this i32)
    local.get $this
    local.get $this
    i32.load offset=8
    i32.const 1
    i32.add
    i32.store offset=8
    local.get $this
    local.get $this
    call $MyApp_build
    call $render
  )
  (func $MyApp_handleSpecsToggle (export "MyApp_handleSpecsToggle")
    (param $this i32)
    (param $checked i32)
    local.get $this
    local.get $checked
    i32.store offset=12
    local.get $this
    local.get $this
    call $MyApp_build
    call $render
  )
  (func $MyApp_handleFFIAlert (export "MyApp_handleFFIAlert")
    (param $this i32)
    (local $tmp_ui_0 i32)
    i32.const 1713
    local.get $this
    i32.load offset=8
    call $concat_int
    call $triggerBrowserAlert
  )
  (func $MyApp_handleFFITime (export "MyApp_handleFFITime")
    (param $this i32)
    (local $epoch i32)
    (local $tmp_ui_0 i32)
    call $getBrowserTime
    local.set $epoch
    local.get $this
    i32.const 1750
    local.get $epoch
    call $concat_int
    i32.store offset=24
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
    i32.const 1775
    call $println
    call $MyApp
    local.set $app
    local.get $app
    local.get $app
    call $MyApp_build
    call $render
    i32.const 1817
    call $println
  )
  (data (i32.const 1817) "Zenith Application Shutdown.\00")
  (data (i32.const 1775) "Initializing Custom Zenith Application...\00")
  (data (i32.const 1705) "checked\00")
  (data (i32.const 1681) "MyApp_handleSpecsToggle\00")
  (data (i32.const 1439) "JavaScript FFI Verification\00")
  (data (i32.const 1583) "Interactive Controls\00")
  (data (i32.const 1235) "Press 'Get Time' to run FFI\00")
  (data (i32.const 1208) "Developer\00")
  (data (i32.const 1538) "Get Browser Time via FFI\00")
  (data (i32.const 1517) "MyApp_handleFFIAlert\00")
  (data (i32.const 1499) "Trigger FFI Alert\00")
  (data (i32.const 1163) "macOS Native Application\00")
  (data (i32.const 1750) "Unix Time from Browser: \00")
  (data (i32.const 1188) "Zenith Web FFI Demo\00")
  (data (i32.const 1310) "Hello, \00")
  (data (i32.const 1620) "Click Me!\00")
  (data (i32.const 1111) "Linux Native Application\00")
  (data (i32.const 1263) "Welcome to \00")
  (data (i32.const 1400) "Enter name here\00")
  (data (i32.const 1095) "Web Application\00")
  (data (i32.const 1084) "iOS Client\00")
  (data (i32.const 1630) "MyApp_handleIncrement\00")
  (data (i32.const 1136) "Windows Native Application\00")
  (data (i32.const 1069) "Android Client\00")
  (data (i32.const 1416) "MyApp_handleNameChange\00")
  (data (i32.const 1042) "Desktop Native Application\00")
  (data (i32.const 1604) "yellow\00")
  (data (i32.const 1275) "fontWeight\00")
  (data (i32.const 1378) "max\00")
  (data (i32.const 1218) "No data fetched.\00")
  (data (i32.const 1291) "color\00")
  (data (i32.const 1491) "magenta\00")
  (data (i32.const 1318) "!\00")
  (data (i32.const 1611) "Clicks: \00")
  (data (i32.const 1563) "MyApp_handleFFITime\00")
  (data (i32.const 1297) "cyan\00")
  (data (i32.const 1652) "Show Platform Module Details\00")
  (data (i32.const 1024) "Shared Code Block\00")
  (data (i32.const 1302) "padding\00")
  (data (i32.const 1286) "bold\00")
  (data (i32.const 1320) "green\00")
  (data (i32.const 1713) "Hello from Zenith! Current count is \00")
  (data (i32.const 1326) "Adjust greeting size: \00")
  (data (i32.const 1349) "MyApp_handleSliderChange\00")
  (data (i32.const 1382) "value\00")
  (data (i32.const 1467) "WASM/JS Bridge Status: \00")
  (data (i32.const 1388) "Type name: \00")
  (data (i32.const 1374) "min\00")
)
