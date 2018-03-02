
IDC_STATIC_TITLE = 1001;
IDC_CHECK_ENABLE = 1002;
IDC_RADIO_FIRST = 1003;
IDC_RADIO_SECOND = 1004;
IDC_EDIT_INPUT = 1006;
IDC_BUTTON_OK = 1007;
IDC_BUTTON_CANCEL = 1008;


title = "test plugin settings";
icon = "";
layout = {
    { type = "static", id = IDC_STATIC_TITLE, text = "this is a static ctrl", x = 10, y = 10 }
    { type = "check", id = IDC_CHECK_ENABLE, text = "this is a checkbox ctrl", x = 10, y = 30 }
    { type = "radio", id = IDC_RADIO_FIRST, text = "this is a radio ctrl 0", x = 30, y = 50 }
    { type = "radio", id = IDC_RADIO_SECOND, text = "this is a radio ctrl 1", x = 30, y = 70 }
    { type = "edit", id = IDC_EDIT_INPUT, text = "this is a edit ctrl", x = 10, y = 90 }
    { type = "button", id = IDC_BUTTON_OK, text = "ok", x = 100, y = 150 }
    { type = "button", id = IDC_BUTTON_CANCEL, text = "cancel", x = 120, y = 150 }
};