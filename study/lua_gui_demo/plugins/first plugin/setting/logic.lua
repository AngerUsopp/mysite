local info = debug.getinfo(1, "S"); -- �ڶ������� "S" ��ʾ������ source,short_src���ֶΣ� ���������� "n", "f", "I", "L"�� ���ز�ͬ���ֶ���Ϣ 
local path = info.source;
path = string.sub(path, 2, -1) -- ȥ����ͷ��"@"    
path = string.match(path, "^.*\\") -- �������һ�� "/" ֮ǰ�Ĳ��� ������������Ҫ��Ŀ¼����
dofile(path .. "ids.lua");  -- ��Ȼ���ԡ����á�������Ҫ���ز�ִ�����ݽű�

gui_proxy = require("lua_gui_proxy");

IDB_BUTTON_0 = "IDB_BUTTON_0";
IDB_BUTTON_1 = "IDB_BUTTON_1";
IDB_BUTTON_2 = "IDB_BUTTON_2";
IDB_BUTTON_3 = "IDB_BUTTON_3";

if buttons == nil then
    buttons = {}
end
buttons = {
    { id = IDB_BUTTON_0, text = "button 0", x = 10, y = 10, width = 80, height = 40 },
    { id = IDB_BUTTON_1, text = "button 1", x = 100, y = 10, width = 80, height = 40 },
    { id = IDB_BUTTON_2, text = "button 2", x = 190, y = 10, width = 80, height = 40 },
    { id = IDB_BUTTON_3, text = "button 3", x = 280, y = 10, width = 80, height = 40 }
};

function OnMouseMove(x, y)
    --return ("lua OnMouseMove");
end

function OnMousePressed(x, y)
    return ("lua OnMousePressed");
end

function OnMouseExit()
    return ("lua OnMouseExit");
end

function OnMenuSelected(id)
    if id == IDM_FIRST_0 then
        return ("lua OnMenuSelected id = 'IDM_FIRST_0'");
    end
    if id == IDM_FIRST_1 then
        return ("lua OnMenuSelected id = 'IDM_FIRST_1'");
    end
    if id == IDM_SECOND0 then
        return ("lua OnMenuSelected id = 'IDM_SECOND0'");
    end
    if id == IDM_SECOND1 then
        return ("lua Menu id = 'IDM_SECOND1'");
    end

    return "no impl menu id = '" .. id .. "'";
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

function main(lua)
    return gui_proxy:domodal();
end