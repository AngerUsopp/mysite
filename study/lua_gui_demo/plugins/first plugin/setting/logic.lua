--������һ��lua�ļ��ķ�ʽdofile��require
--[[��dofile��ʽ�����á���Ҫ�õ�Ŀ���ļ��ľ���·�������һ�ִ��һ��ű���Ϊ�˱���ִ�в���Ҫ�Ĳ�����
��Ŀ���ļ�����Ӧ�ý���ֻ�Ƕ������/��������Ӧ���κκ������ò���
]]--
local info = debug.getinfo(1, "S"); -- �ڶ������� "S" ��ʾ������ source,short_src���ֶΣ� ���������� "n", "f", "I", "L"�� ���ز�ͬ���ֶ���Ϣ 
local path = info.source;
path = string.sub(path, 2, -1) -- ȥ����ͷ��"@"    
path = string.match(path, "^.*\\") -- �������һ�� "/" ֮ǰ�Ĳ��� ������������Ҫ��Ŀ¼����
dofile(path .. "layout.lua");  -- ��Ȼ���ԡ����á�������Ҫ���ز�ִ�����ݽű�

--[[��require��ʽ�����������ļ�������ִ��һ�顣������lua�ļ���˳����Ӧlua�ļ����������������·���£�
����ļ�������һ����㼶��Ŀ¼��������Ŀ¼��������������Ŀ¼�£�����lua�ļ��ڲ���Ҫȷ��֪���Լ�������
������������·�������������΢�Ķ�һ��Ŀ¼�ṹ�������һһ�޸����ڲ�ȫ����lua�ļ�����
]]--
--require("plugins\\test plugin\\layout");   -- ֱ������lua�ļ���Ҫȷ��֪��/��ȡĿ���ļ�������exe�����·��������ִ�нű�


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
