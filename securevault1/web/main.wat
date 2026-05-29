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
  (import "env" "deriveKey" (func $deriveKey (param i32) (param i32) (result i32)))
  (memory (export "memory") 1)
  (global $free_mem (mut i32) (i32.const 8192))

  (func $VaultService (export "VaultService")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 1024
    i32.store offset=0
    global.get $free_mem
    i32.const 4
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $VaultService_unlock (export "VaultService_unlock")
    (param $this i32)
    (param $password i32)
    (local $salt i32)
    i32.const 1025
    local.set $salt
    local.get $this
    local.get $password
    local.get $salt
    call $deriveKey
    i32.store offset=0
  )
  (func $VaultService_save (export "VaultService_save")
    (param $this i32)
    (param $noteText i32)
    (result i32)
    (local $tmp_ui_0 i32)
    local.get $noteText
    local.get $this
    i32.load offset=0
    call $encrypt
    return
  )
  (func $VaultService_open (export "VaultService_open")
    (param $this i32)
    (param $cipherText i32)
    (result i32)
    (local $tmp_ui_0 i32)
    local.get $cipherText
    local.get $this
    i32.load offset=0
    call $decrypt
    return
  )
  (func $VaultService_summarize (export "VaultService_summarize")
    (param $this i32)
    (param $noteText i32)
    (result i32)
    local.get $noteText
    call $summarizeDocument
    return
  )
  (func $NotesApp (export "NotesApp")
    (result i32)
    (local $this i32)
    global.get $free_mem
    local.set $this
    local.get $this
    i32.const 1024
    i32.store offset=0
    local.get $this
    i32.const 1024
    i32.store offset=4
    local.get $this
    i32.const 1024
    i32.store offset=8
    local.get $this
    i32.const 1049
    i32.store offset=12
    local.get $this
    i32.const 1024
    i32.store offset=16
    local.get $this
    i32.const 1024
    i32.store offset=20
    local.get $this
    i32.const 1024
    i32.store offset=24
    global.get $free_mem
    i32.const 28
    i32.add
    global.set $free_mem
    local.get $this
    return
  )

  (func $NotesApp_build (export "NotesApp_build")
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
    call $create_column
    local.set $tmp_ui_0
    local.get $tmp_ui_0
    i32.const 1056
    call $create_text
    local.set $tmp_ui_1
    local.get $tmp_ui_1
    i32.const 1086
    i32.const 1097
    call $set_attribute
    local.get $tmp_ui_1
    i32.const 1102
    i32.const 1108
    call $set_attribute
    local.get $tmp_ui_1
    call $append_child
    local.get $tmp_ui_0
    i32.const 1113
    local.get $this
    i32.load offset=12
    call $concat_int
    call $create_text
    local.set $tmp_ui_2
    local.get $tmp_ui_2
    i32.const 1102
    i32.const 1122
    call $set_attribute
    local.get $tmp_ui_2
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_3
    local.get $tmp_ui_3
    i32.const 1128
    call $create_text
    local.set $tmp_ui_4
    local.get $tmp_ui_4
    call $append_child
    local.get $tmp_ui_3
    call $create_row
    local.set $tmp_ui_5
    local.get $tmp_ui_5
    i32.const 1154
    call $create_text
    local.set $tmp_ui_6
    local.get $tmp_ui_6
    call $append_child
    local.get $tmp_ui_5
    i32.const 0
    local.get $this
    i32.const 1164
    call $create_textfield
    local.set $tmp_ui_7
    local.get $tmp_ui_7
    i32.const 1194
    local.get $this
    i32.load offset=4
    call $set_attribute_int
    local.get $tmp_ui_7
    i32.const 1200
    i32.const 1212
    call $set_attribute
    local.get $tmp_ui_7
    call $append_child
    local.get $tmp_ui_5
    i32.const 1227
    local.get $this
    i32.const 1234
    call $create_button
    local.set $tmp_ui_8
    local.get $tmp_ui_8
    call $append_child
    local.get $tmp_ui_5
    call $append_child
    local.get $tmp_ui_3
    i32.const 1256
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_3
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_9
    local.get $tmp_ui_9
    i32.const 1264
    call $create_text
    local.set $tmp_ui_10
    local.get $tmp_ui_10
    call $append_child
    local.get $tmp_ui_9
    call $create_row
    local.set $tmp_ui_11
    local.get $tmp_ui_11
    i32.const 1288
    call $create_text
    local.set $tmp_ui_12
    local.get $tmp_ui_12
    call $append_child
    local.get $tmp_ui_11
    i32.const 0
    local.get $this
    i32.const 1299
    call $create_textfield
    local.set $tmp_ui_13
    local.get $tmp_ui_13
    i32.const 1194
    local.get $this
    i32.load offset=8
    call $set_attribute_int
    local.get $tmp_ui_13
    i32.const 1200
    i32.const 1325
    call $set_attribute
    local.get $tmp_ui_13
    call $append_child
    local.get $tmp_ui_11
    call $append_child
    local.get $tmp_ui_9
    call $create_row
    local.set $tmp_ui_14
    local.get $tmp_ui_14
    i32.const 1343
    local.get $this
    i32.const 1358
    call $create_button
    local.set $tmp_ui_15
    local.get $tmp_ui_15
    call $append_child
    local.get $tmp_ui_14
    i32.const 1378
    local.get $this
    i32.const 1391
    call $create_button
    local.set $tmp_ui_16
    local.get $tmp_ui_16
    call $append_child
    local.get $tmp_ui_14
    i32.const 1414
    local.get $this
    i32.const 1429
    call $create_button
    local.set $tmp_ui_17
    local.get $tmp_ui_17
    call $append_child
    local.get $tmp_ui_14
    call $append_child
    local.get $tmp_ui_9
    i32.const 1256
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_9
    call $append_child
    local.get $tmp_ui_0
    call $create_card
    local.set $tmp_ui_18
    local.get $tmp_ui_18
    i32.const 1454
    call $create_text
    local.set $tmp_ui_19
    local.get $tmp_ui_19
    call $append_child
    local.get $tmp_ui_18
    local.get $this
    i32.load offset=24
    call $create_text
    local.set $tmp_ui_20
    local.get $tmp_ui_20
    i32.const 1102
    i32.const 1473
    call $set_attribute
    local.get $tmp_ui_20
    call $append_child
    local.get $tmp_ui_18
    i32.const 1480
    call $create_text
    local.set $tmp_ui_21
    local.get $tmp_ui_21
    call $append_child
    local.get $tmp_ui_18
    local.get $this
    i32.load offset=16
    call $create_text
    local.set $tmp_ui_22
    local.get $tmp_ui_22
    i32.const 1102
    i32.const 1122
    call $set_attribute
    local.get $tmp_ui_22
    call $append_child
    local.get $tmp_ui_18
    i32.const 1496
    call $create_text
    local.set $tmp_ui_23
    local.get $tmp_ui_23
    call $append_child
    local.get $tmp_ui_18
    local.get $this
    i32.load offset=20
    call $create_text
    local.set $tmp_ui_24
    local.get $tmp_ui_24
    i32.const 1102
    i32.const 1508
    call $set_attribute
    local.get $tmp_ui_24
    call $append_child
    local.get $tmp_ui_18
    i32.const 1256
    i32.const 1
    call $set_attribute_int
    local.get $tmp_ui_18
    call $append_child
    local.get $tmp_ui_0
    return
  )
  (func $NotesApp_handlePasswordChange (export "NotesApp_handlePasswordChange")
    (param $this i32)
    (param $val i32)
    local.get $this
    local.get $val
    i32.store offset=4
  )
  (func $NotesApp_handleNoteChange (export "NotesApp_handleNoteChange")
    (param $this i32)
    (param $val i32)
    local.get $this
    local.get $val
    i32.store offset=8
  )
  (func $NotesApp_handleUnlock (export "NotesApp_handleUnlock")
    (param $this i32)
    (local $vault i32)
    (local $tmp_ui_0 i32)
    call $VaultService
    local.set $vault
    local.get $vault
    local.get $this
    i32.load offset=4
    call $VaultService_unlock
    local.get $this
    i32.const 1516
    i32.store offset=12
    local.get $this
    local.get $this
    call $NotesApp_build
    call $render
  )
  (func $NotesApp_handleSave (export "NotesApp_handleSave")
    (param $this i32)
    (local $vault i32)
    (local $encrypted i32)
    (local $tmp_ui_0 i32)
    (local $tmp_ui_1 i32)
    call $VaultService
    local.set $vault
    local.get $vault
    local.get $this
    i32.load offset=4
    call $VaultService_unlock
    local.get $vault
    local.get $this
    i32.load offset=8
    call $VaultService_save
    local.set $encrypted
    local.get $this
    local.get $encrypted
    i32.store offset=24
    local.get $this
    i32.const 1024
    i32.store offset=16
    local.get $this
    i32.const 1024
    i32.store offset=20
    local.get $this
    local.get $this
    call $NotesApp_build
    call $render
    i32.const 1525
    local.get $encrypted
    call $concat_int
    call $println
  )
  (func $NotesApp_handleDecrypt (export "NotesApp_handleDecrypt")
    (param $this i32)
    (local $vault i32)
    (local $decrypted i32)
    (local $tmp_ui_0 i32)
    call $VaultService
    local.set $vault
    local.get $vault
    local.get $this
    i32.load offset=4
    call $VaultService_unlock
    local.get $vault
    local.get $this
    i32.load offset=24
    call $VaultService_open
    local.set $decrypted
    local.get $this
    local.get $decrypted
    i32.store offset=16
    local.get $this
    local.get $this
    call $NotesApp_build
    call $render
  )
  (func $NotesApp_handleSummarize (export "NotesApp_handleSummarize")
    (param $this i32)
    (local $vault i32)
    (local $summary i32)
    (local $tmp_ui_0 i32)
    call $VaultService
    local.set $vault
    local.set $summary
    local.get $this
    local.get $summary
    i32.store offset=20
    local.get $this
    local.get $this
    call $NotesApp_build
    call $render
  )
  (func $main (export "main")
    (local $app i32)
    (local $tmp_ui_0 i32)
    call $NotesApp
    local.set $app
    local.get $app
    local.get $app
    call $NotesApp_build
    call $render
  )
  (data (i32.const 1516) "Unlocked\00")
  (data (i32.const 1525) "Saved Encrypted: \00")
  (data (i32.const 1508) "magenta\00")
  (data (i32.const 1496) "AI Summary:\00")
  (data (i32.const 1480) "Decrypted Note:\00")
  (data (i32.const 1128) "1. Authenticate / Unlock:\00")
  (data (i32.const 1113) "Status: \00")
  (data (i32.const 1194) "value\00")
  (data (i32.const 1108) "cyan\00")
  (data (i32.const 1200) "placeholder\00")
  (data (i32.const 1102) "color\00")
  (data (i32.const 1154) "Password:\00")
  (data (i32.const 1473) "yellow\00")
  (data (i32.const 1086) "fontWeight\00")
  (data (i32.const 1264) "2. Secure Notes Editor:\00")
  (data (i32.const 1056) "SecureVault - Encrypted Notes\00")
  (data (i32.const 1049) "Locked\00")
  (data (i32.const 1025) "zenith_securevault_salt\00")
  (data (i32.const 1164) "NotesApp_handlePasswordChange\00")
  (data (i32.const 1097) "bold\00")
  (data (i32.const 1122) "green\00")
  (data (i32.const 1358) "NotesApp_handleSave\00")
  (data (i32.const 1234) "NotesApp_handleUnlock\00")
  (data (i32.const 1024) "\00")
  (data (i32.const 1288) "Note text:\00")
  (data (i32.const 1227) "Unlock\00")
  (data (i32.const 1429) "NotesApp_handleSummarize\00")
  (data (i32.const 1256) "padding\00")
  (data (i32.const 1454) "Encrypted Payload:\00")
  (data (i32.const 1299) "NotesApp_handleNoteChange\00")
  (data (i32.const 1343) "Save Encrypted\00")
  (data (i32.const 1378) "Decrypt Note\00")
  (data (i32.const 1212) "Enter Password\00")
  (data (i32.const 1391) "NotesApp_handleDecrypt\00")
  (data (i32.const 1325) "Secret content...\00")
  (data (i32.const 1414) "Summarize (AI)\00")
)
