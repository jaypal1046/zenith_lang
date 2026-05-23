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

  (func $GalleryApp (export "GalleryApp")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 1024
    i32.store offset=0
    local.get $this
    i32.const 1048
    i32.store offset=4
    local.get $this
    i32.const 0
    i32.store offset=8
    global.get $free_mem
    i32.const 12
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $GalleryApp_build (export "GalleryApp_build")
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
    (local $tmp_ui_23 i32)
    (local $tmp_ui_24 i32)
    (local $tmp_ui_25 i32)
    (local $tmp_ui_26 i32)
    (local $tmp_ui_27 i32)
    (local $tmp_ui_28 i32)
    (local $tmp_ui_29 i32)
    (local $tmp_ui_30 i32)
    (local $tmp_ui_31 i32)
    (local $tmp_ui_32 i32)
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    local.get $this
    i32.load offset=0
    call $create_text
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    i32.const 1073
    i32.const 1084
    call $set_attribute
    local.get $tmp_ui_1
    i32.const 1089
    i32.const 1095
    call $set_attribute
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    call $create_container
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1100
    call $create_text
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1073
    i32.const 1084
    call $set_attribute
    local.get $tmp_ui_3
    i32.const 1089
    i32.const 1130
    call $set_attribute
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_2
    call $create_row
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    i32.const 1137
    call $create_text
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    i32.const 1089
    i32.const 1149
    call $set_attribute
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_4
    i32.const 1155
    call $create_text
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    i32.const 1089
    i32.const 1167
    call $set_attribute
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_2
    i32.const 1175
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    i32.const 1183
    call $create_text
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    i32.const 1073
    i32.const 1084
    call $set_attribute
    local.get $tmp_ui_8
    i32.const 1089
    i32.const 1095
    call $set_attribute
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_7
    call $create_row
    local.set $tmp_ui_9
    local.get $tmp_ui_9
    i32.const 1212
    local.get $this
    i32.load offset=8
    call $concat_int
    call $create_text
    local.set $tmp_ui_10
    local.get $tmp_ui_10
    i32.const 1089
    i32.const 1149
    call $set_attribute
    local.get $tmp_ui_10
    call $append_child
    local.get $tmp_ui_9
    i32.const 1233
    local.get $this
    i32.const 1243
    call $create_button
    local.set $tmp_ui_11
    local.get $tmp_ui_11
    call $append_child
    local.get $tmp_ui_9
    call $append_child
    local.get $tmp_ui_7
    i32.const 1175
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_0
    call $create_row
    local.set $tmp_ui_12
    local.get $tmp_ui_12
    i32.const 1266
    call $create_image
    local.set $tmp_ui_13
    local.get $tmp_ui_13
    i32.const 1327
    i32.const 35
    call $set_attribute_int
    local.get $tmp_ui_13
    i32.const 1333
    i32.const 4
    call $set_attribute_int
    local.get $tmp_ui_13
    call $append_child
    local.get $tmp_ui_12
    i32.const 1340
    call $create_video
    local.set $tmp_ui_14
    local.get $tmp_ui_14
    i32.const 1327
    i32.const 35
    call $set_attribute_int
    local.get $tmp_ui_14
    i32.const 1333
    i32.const 4
    call $set_attribute_int
    local.get $tmp_ui_14
    call $append_child
    local.get $tmp_ui_12
    call $append_child
    local.get $tmp_ui_0
    call $create_scrolling
    local.set $tmp_ui_15
    local.get $tmp_ui_15
    i32.const 1428
    call $create_text
    local.set $tmp_ui_16
    local.get $tmp_ui_16
    call $append_child
    local.get $tmp_ui_15
    i32.const 1459
    call $create_text
    local.set $tmp_ui_17
    local.get $tmp_ui_17
    call $append_child
    local.get $tmp_ui_15
    i32.const 1491
    call $create_text
    local.set $tmp_ui_18
    local.get $tmp_ui_18
    call $append_child
    local.get $tmp_ui_15
    i32.const 1522
    call $create_text
    local.set $tmp_ui_19
    local.get $tmp_ui_19
    call $append_child
    local.get $tmp_ui_15
    i32.const 1557
    call $create_text
    local.set $tmp_ui_20
    local.get $tmp_ui_20
    call $append_child
    local.get $tmp_ui_15
    i32.const 1587
    call $create_text
    local.set $tmp_ui_21
    local.get $tmp_ui_21
    call $append_child
    local.get $tmp_ui_15
    i32.const 1333
    i32.const 4
    call $set_attribute_int
    local.get $tmp_ui_15
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_22
    local.get $tmp_ui_22
    i32.const 1625
    call $create_text
    local.set $tmp_ui_23
    local.get $tmp_ui_23
    i32.const 1073
    i32.const 1084
    call $set_attribute
    local.get $tmp_ui_23
    i32.const 1089
    i32.const 1130
    call $set_attribute
    local.get $tmp_ui_23
    call $append_child
    local.get $tmp_ui_22
    call $create_row
    local.set $tmp_ui_24
    local.get $tmp_ui_24
    i32.const 1666
    local.get $this
    i32.load offset=4
    call $concat_int
    call $create_text
    local.set $tmp_ui_25
    local.get $tmp_ui_25
    call $append_child
    local.get $tmp_ui_24
    i32.const 1677
    local.get $this
    i32.const 1688
    call $create_button
    local.set $tmp_ui_26
    local.get $tmp_ui_26
    call $append_child
    local.get $tmp_ui_24
    call $append_child
    local.get $tmp_ui_22
    i32.const 1175
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_22
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_27
    local.get $tmp_ui_27
    i32.const 1709
    call $create_text
    local.set $tmp_ui_28
    local.get $tmp_ui_28
    i32.const 1073
    i32.const 1084
    call $set_attribute
    local.get $tmp_ui_28
    i32.const 1089
    i32.const 1167
    call $set_attribute
    local.get $tmp_ui_28
    call $append_child
    local.get $tmp_ui_27
    call $create_row
    local.set $tmp_ui_29
    local.get $tmp_ui_29
    i32.const 1745
    call $create_text
    local.set $tmp_ui_30
    local.get $tmp_ui_30
    i32.const 1089
    i32.const 1754
    call $set_attribute
    local.get $tmp_ui_30
    i32.const 1758
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_30
    call $append_child
    local.get $tmp_ui_29
    i32.const 1767
    call $create_text
    local.set $tmp_ui_31
    local.get $tmp_ui_31
    i32.const 1089
    i32.const 1149
    call $set_attribute
    local.get $tmp_ui_31
    i32.const 1758
    i32.const 2
    call $set_attribute_int
    local.get $tmp_ui_31
    call $append_child
    local.get $tmp_ui_29
    i32.const 1745
    call $create_text
    local.set $tmp_ui_32
    local.get $tmp_ui_32
    i32.const 1089
    i32.const 1776
    call $set_attribute
    local.get $tmp_ui_32
    i32.const 1758
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_32
    call $append_child
    local.get $tmp_ui_29
    i32.const 1781
    i32.const 1796
    call $set_attribute
    local.get $tmp_ui_29
    i32.const 1809
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_29
    i32.const 1327
    i32.const 35
    call $set_attribute_int
    local.get $tmp_ui_29
    call $append_child
    local.get $tmp_ui_27
    i32.const 1175
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_27
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $GalleryApp_handlePress (export "GalleryApp_handlePress")
    (param $this i32)
    local.get $this
    local.get $this
    i32.load offset=8
    i32.const 1
    i32.add
    i32.store offset=8
    local.get $this
    local.get $this
    call $GalleryApp_build
    call $render
  )
  (func $GalleryApp_fetchData (export "GalleryApp_fetchData")
    (param $this i32)
    (local $res i32)
    (local $tmp_ui_0 i32)
    i32.const 1813
    call $httpGet
    local.set $res
    local.get $this
    local.get $res
    i32.store offset=4
    local.get $this
    local.get $this
    call $GalleryApp_build
    call $render
  )
  (func $main (export "main")
    (local $app i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    (local $tmp_ui_2 i32)
    i32.const 1856
    call $println
    call $GalleryApp
    local.set $app
    local.get $app
    local.get $app
    call $GalleryApp_build
    call $render
    i32.const 1906
    call $println
  )
  (data (i32.const 1906) "--- Gallery Loaded Successfully. Interactive loop started! ---\00")
  (data (i32.const 1856) "--- Booting Zenith Multi-Platform Gallery App ---\00")
  (data (i32.const 1813) "https://jsonplaceholder.typicode.com/users\00")
  (data (i32.const 1767) "[Grow 2]\00")
  (data (i32.const 1758) "flexGrow\00")
  (data (i32.const 1754) "red\00")
  (data (i32.const 1688) "GalleryApp_fetchData\00")
  (data (i32.const 1776) "blue\00")
  (data (i32.const 1428) "Line 1: Infinite Scrolling Row\00")
  (data (i32.const 1175) "padding\00")
  (data (i32.const 1095) "cyan\00")
  (data (i32.const 1024) "ZENITH PLATFORM GALLERY\00")
  (data (i32.const 1522) "Line 4: Native Platform Primitives\00")
  (data (i32.const 1089) "color\00")
  (data (i32.const 1137) "Flow item 1\00")
  (data (i32.const 1183) "2. Reactive Component States\00")
  (data (i32.const 1155) "Flow item 2\00")
  (data (i32.const 1130) "yellow\00")
  (data (i32.const 1073) "fontWeight\00")
  (data (i32.const 1796) "space-around\00")
  (data (i32.const 1243) "GalleryApp_handlePress\00")
  (data (i32.const 1677) "Fetch JSON\00")
  (data (i32.const 1149) "green\00")
  (data (i32.const 1084) "bold\00")
  (data (i32.const 1048) "No API data fetched yet.\00")
  (data (i32.const 1167) "magenta\00")
  (data (i32.const 1745) "[Grow 1]\00")
  (data (i32.const 1333) "height\00")
  (data (i32.const 1491) "Line 3: Reactive State Binding\00")
  (data (i32.const 1666) "Response: \00")
  (data (i32.const 1809) "gap\00")
  (data (i32.const 1212) "Interactive clicks: \00")
  (data (i32.const 1709) "4. CSS Flexbox Layout Demonstration\00")
  (data (i32.const 1233) "Click Me!\00")
  (data (i32.const 1266) "https://images.unsplash.com/photo-1618005182384-a83a8bd57fbe\00")
  (data (i32.const 1781) "justifyContent\00")
  (data (i32.const 1327) "width\00")
  (data (i32.const 1459) "Line 2: GPU Accelerated Layouts\00")
  (data (i32.const 1340) "https://assets.mixkit.co/videos/preview/mixkit-stars-in-space-background-1611-large.mp4\00")
  (data (i32.const 1100) "1. Widget Grid & Flow Layouts\00")
  (data (i32.const 1557) "Line 5: Dynamic CSS Variables\00")
  (data (i32.const 1587) "Line 6: Single-Thread WASM Event Loop\00")
  (data (i32.const 1625) "3. REST API / Network Client Integration\00")
)
