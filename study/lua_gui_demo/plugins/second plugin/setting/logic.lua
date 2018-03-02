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


function main(lua)
    ret = gui_proxy:DoModal(lua);
    return (ret == 1);
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
