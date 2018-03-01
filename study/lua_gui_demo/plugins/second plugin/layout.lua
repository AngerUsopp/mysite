ids = require("ids.lua");

title = "test plugin settings";
icon = "";
layout = {
    { type = "static", id = ids.IDC_STATIC_TITLE, text = "this is a static ctrl", x = 10, y = 10 }
    { type = "check", id = ids.IDC_CHECK_ENABLE, text = "this is a checkbox ctrl", x = 10, y = 30 }
    { type = "radio", id = ids.IDC_RADIO_FIRST, text = "this is a radio ctrl 0", x = 30, y = 50 }
    { type = "radio", id = ids.IDC_RADIO_SECOND, text = "this is a radio ctrl 1", x = 30, y = 70 }
    { type = "edit", id = ids.IDC_EDIT_INPUT, text = "this is a edit ctrl", x = 10, y = 90 }
    { type = "button", id = ids.IDC_BUTTON_OK, text = "ok", x = 100, y = 150 }
    { type = "button", id = ids.IDC_BUTTON_CANCEL, text = "cancel", x = 120, y = 150 }
};