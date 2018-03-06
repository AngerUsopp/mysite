--引用另一个lua文件的方式dofile、require
--[[以dofile方式“引用”需要得到目标文件的绝对路径，并且会执行一遍脚本，为了避免执行不必要的操作，
则目标文件里面应该仅仅只是定义变量/函数，不应有任何函数调用操作
]]--
local info = debug.getinfo(1, "S"); -- 第二个参数 "S" 表示仅返回 source,short_src等字段， 其他还可以 "n", "f", "I", "L"等 返回不同的字段信息 
local path = info.source;
path = string.sub(path, 2, -1) -- 去掉开头的"@"    
path = string.match(path, "^.*\\") -- 捕获最后一个 "/" 之前的部分 就是我们最终要的目录部分
dofile(path .. "layout.lua");  -- 虽然可以“引用”，但需要加载并执行整份脚本

--[[以require方式能真正引用文件，无需执行一遍。但根据lua的检索顺序，相应lua文件必须放在宿主程序路径下，
如果文件本身在一个多层级的目录中则整个目录必须在宿主程序目录下，并且lua文件内部需要确切知道自己与宿主
程序的完整相对路径，今后若是稍微改动一下目录结构，则必须一一修改其内部全部的lua文件代码
]]--
--require("plugins\\test plugin\\layout");   -- 直接引用lua文件需要确切知道/获取目标文件与宿主exe的相对路径，不会执行脚本


gui_proxy = require("lua_gui_proxy");
local widget = nil;

function main(lua)
    widget = gui_proxy:CreateWidget(lua);
    ret = widget:DoModal(lua);
    widget = nil;

    collectgarbage("collect");

    return ret;
end

function OnKickIdle()
    if widget ~= nil then
        if widget:IsDlgButtonChecked(IDC_CHECK_ENABLE) == true then
            widget:SetDlgItemEnable(IDC_RADIO_FIRST, true);
            widget:SetDlgItemEnable(IDC_RADIO_SECOND, true);
        else
            widget:SetDlgItemEnable(IDC_RADIO_FIRST, false);
            widget:SetDlgItemEnable(IDC_RADIO_SECOND, false);
        end
    end
end

function OnCommand(id, code)
    if widget ~= nil then
        if id == IDC_BUTTON_OK then
            --widget:EndDialog(1);
            local cb = widget:IsDlgButtonChecked(IDC_CHECK_ENABLE);
            if cb then
                cbt = "true";
            else
                cbt = "false";
            end
            local r1 = widget:IsDlgButtonChecked(IDC_RADIO_FIRST);
            if r1 then
                rt1 = "true";
            else
                rt1 = "false";
            end
            local r2 = widget:IsDlgButtonChecked(IDC_RADIO_SECOND);
            if cb then
                rt2 = "true";
            else
                rt2 = "false";
            end
            local edit_text = widget:Edit_GetText(IDC_EDIT_INPUT);
            gui_proxy:MessageBox(string.format("checkbox=%s, radiofirst=%s, radiosecond=%s, edittext='%s'", 
                cbt, rt1, rt2, edit_text));
        elseif id == IDC_BUTTON_CANCEL then
            widget:EndDialog(2);
        else
            --gui_proxy:MessageBox(string.format("lua OnCommand(%d, %d)", id, code));
        end
    end
end

function OnMouseMove(x, y)
    --return ("lua OnMouseMove");
end

function OnMousePressed(x, y)
    return ("lua OnMousePressed");
end

function OnMouseExit()
    return ("lua OnMouseExit");
end

function OnButtonClick(id)
    if id == IDB_BUTTON_0 then
        return ("lua btn click id = 'IDB_BUTTON_0'");
    end
    if id == IDB_BUTTON_1 then
        return ("lua btn click id = 'IDB_BUTTON_1'");
    end
    if id == IDB_BUTTON_2 then
        return ("lua btn click id = 'IDB_BUTTON_2'");
    end
    if id == IDB_BUTTON_3 then
        return ("lua btn click id = 'IDB_BUTTON_3'");
    end

    return "no impl btn id = '" .. id .. "'";
end
