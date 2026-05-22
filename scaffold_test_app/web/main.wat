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
    i32.const 0
    i32.store offset=4
    local.get $this
    i32.const 1214
    i32.store offset=8
    local.get $this
    call $AppCommon
    i32.store offset=12
    local.get $this
    call $AppDesktop
    i32.store offset=16
    local.get $this
    call $AppAndroid
    i32.store offset=20
    local.get $this
    call $AppIos
    i32.store offset=24
    local.get $this
    call $AppWeb
    i32.store offset=28
    local.get $this
    call $AppLinux
    i32.store offset=32
    local.get $this
    call $AppWindows
    i32.store offset=36
    local.get $this
    call $AppMac
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
    (local $tmp_ui_22 i32)
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    call $create_container
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    local.get $this
    i32.load offset=0
    i32.const 1231
    local.get $this
    i32.load offset=12
    call $getCommonMessage
    i32.const 1234
    i32.add
    call $concat_int
    i32.add
    call $create_text
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1236
    i32.const 1247
    call $set_attribute
    local.get $tmp_ui_2
    i32.const 1252
    i32.const 1258
    call $set_attribute
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_1
    i32.const 1263
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1271
    call $create_text
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    i32.const 1236
    i32.const 1247
    call $set_attribute
    local.get $tmp_ui_4
    i32.const 1252
    i32.const 1296
    call $set_attribute
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_3
    i32.const 1301
    local.get $this
    i32.load offset=12
    call $getCommonMessage
    call $concat_int
    call $create_text
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_3
    i32.const 1313
    local.get $this
    i32.load offset=16
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_3
    i32.const 1340
    local.get $this
    i32.load offset=20
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_3
    i32.const 1367
    local.get $this
    i32.load offset=24
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_3
    i32.const 1390
    local.get $this
    i32.load offset=28
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_9
    local.get $tmp_ui_9
    call $append_child
    local.get $tmp_ui_3
    i32.const 1413
    local.get $this
    i32.load offset=32
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_10
    local.get $tmp_ui_10
    call $append_child
    local.get $tmp_ui_3
    i32.const 1438
    local.get $this
    i32.load offset=36
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_11
    local.get $tmp_ui_11
    call $append_child
    local.get $tmp_ui_3
    i32.const 1465
    local.get $this
    i32.load offset=40
    call $getPlatformName
    call $concat_int
    call $create_text
    local.set $tmp_ui_12
    local.get $tmp_ui_12
    call $append_child
    local.get $tmp_ui_3
    i32.const 1263
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_13
    local.get $tmp_ui_13
    i32.const 1490
    call $create_text
    local.set $tmp_ui_14
    local.get $tmp_ui_14
    i32.const 1236
    i32.const 1247
    call $set_attribute
    local.get $tmp_ui_14
    i32.const 1252
    i32.const 1511
    call $set_attribute
    local.get $tmp_ui_14
    call $append_child
    local.get $tmp_ui_13
    call $create_row
    local.set $tmp_ui_15
    local.get $tmp_ui_15
    i32.const 1518
    local.get $this
    i32.load offset=4
    call $concat_int
    call $create_text
    local.set $tmp_ui_16
    local.get $tmp_ui_16
    i32.const 1252
    i32.const 1526
    call $set_attribute
    local.get $tmp_ui_16
    call $append_child
    local.get $tmp_ui_15
    i32.const 1532
    local.get $this
    i32.const 1542
    call $create_button
    local.set $tmp_ui_17
    local.get $tmp_ui_17
    call $append_child
    local.get $tmp_ui_15
    call $append_child
    local.get $tmp_ui_13
    i32.const 1263
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_13
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_18
    local.get $tmp_ui_18
    i32.const 1564
    call $create_text
    local.set $tmp_ui_19
    local.get $tmp_ui_19
    i32.const 1236
    i32.const 1247
    call $set_attribute
    local.get $tmp_ui_19
    i32.const 1252
    i32.const 1594
    call $set_attribute
    local.get $tmp_ui_19
    call $append_child
    local.get $tmp_ui_18
    call $create_row
    local.set $tmp_ui_20
    local.get $tmp_ui_20
    i32.const 1602
    local.get $this
    i32.load offset=8
    call $concat_int
    call $create_text
    local.set $tmp_ui_21
    local.get $tmp_ui_21
    call $append_child
    local.get $tmp_ui_20
    i32.const 1616
    local.get $this
    i32.const 1627
    call $create_button
    local.set $tmp_ui_22
    local.get $tmp_ui_22
    call $append_child
    local.get $tmp_ui_20
    call $append_child
    local.get $tmp_ui_18
    i32.const 1263
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_18
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
    i32.const 1645
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
    i32.const 1690
    call $println
    call $MyApp
    local.set $app
    local.get $app
    local.get $app
    call $MyApp_build
    call $render
    i32.const 1737
    call $println
  )
  (data (i32.const 1690) "Initializing Reorganized Zenith Application...\00")
  (data (i32.const 1627) "MyApp_handleFetch\00")
  (data (i32.const 1594) "magenta\00")
  (data (i32.const 1737) "Zenith Application Shutdown.\00")
  (data (i32.const 1616) "Fetch Data\00")
  (data (i32.const 1564) "REST Network API Verification\00")
  (data (i32.const 1532) "Increment\00")
  (data (i32.const 1231) " (\00")
  (data (i32.const 1542) "MyApp_handleIncrement\00")
  (data (i32.const 1136) "Windows Native Application\00")
  (data (i32.const 1084) "iOS Client\00")
  (data (i32.const 1340) " - Android module target: \00")
  (data (i32.const 1367) " - iOS module target: \00")
  (data (i32.const 1188) "Zenith Cross-Platform App\00")
  (data (i32.const 1163) "macOS Native Application\00")
  (data (i32.const 1390) " - Web module target: \00")
  (data (i32.const 1602) "API Payload: \00")
  (data (i32.const 1111) "Linux Native Application\00")
  (data (i32.const 1095) "Web Application\00")
  (data (i32.const 1042) "Desktop Native Application\00")
  (data (i32.const 1518) "Value: \00")
  (data (i32.const 1069) "Android Client\00")
  (data (i32.const 1526) "green\00")
  (data (i32.const 1247) "bold\00")
  (data (i32.const 1214) "No data fetched.\00")
  (data (i32.const 1252) "color\00")
  (data (i32.const 1465) " - macOS module target: \00")
  (data (i32.const 1258) "cyan\00")
  (data (i32.const 1024) "Shared Code Block\00")
  (data (i32.const 1263) "padding\00")
  (data (i32.const 1490) "Click Counter Sample\00")
  (data (i32.const 1271) "Platform Modules Loaded:\00")
  (data (i32.const 1413) " - Linux module target: \00")
  (data (i32.const 1236) "fontWeight\00")
  (data (i32.const 1511) "yellow\00")
  (data (i32.const 1645) "https://jsonplaceholder.typicode.com/todos/1\00")
  (data (i32.const 1234) ")\00")
  (data (i32.const 1296) "blue\00")
  (data (i32.const 1301) " - Common: \00")
  (data (i32.const 1313) " - Desktop module target: \00")
  (data (i32.const 1438) " - Windows module target: \00")
)
