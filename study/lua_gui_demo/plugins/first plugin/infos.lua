--������һ��lua�ļ��ķ�ʽdofile��require
--[[��dofile��ʽ�����á���Ҫ�õ�Ŀ���ļ��ľ���·�������һ�ִ��һ��ű���Ϊ�˱���ִ�в���Ҫ�Ĳ�����
��Ŀ���ļ�����Ӧ�ý���ֻ�Ƕ������/��������Ӧ���κκ������ò���
]]--
local info = debug.getinfo(1, "S"); -- �ڶ������� "S" ��ʾ������ source,short_src���ֶΣ� ���������� "n", "f", "I", "L"�� ���ز�ͬ���ֶ���Ϣ 
local path = info.source;
path = string.sub(path, 2, -1) -- ȥ����ͷ��"@"    
path = string.match(path, "^.*\\") -- �������һ�� "/" ֮ǰ�Ĳ��� ������������Ҫ��Ŀ¼����
dofile(path .. "ids.lua");  -- ��Ȼ���ԡ����á�������Ҫ���ز�ִ�����ݽű�

--[[��require��ʽ�����������ļ�������ִ��һ�顣������lua�ļ���˳����Ӧlua�ļ����������������·���£�
����ļ�������һ����㼶��Ŀ¼��������Ŀ¼��������������Ŀ¼�£�����lua�ļ��ڲ���Ҫȷ��֪���Լ�������
������������·�������������΢�Ķ�һ��Ŀ¼�ṹ�������һһ�޸����ڲ�ȫ����lua�ļ�����
]]--
--require("plugins\\test plugin\\ids");   -- ֱ������lua�ļ���Ҫȷ��֪��/��ȡĿ���ļ�������exe�����·��������ִ�нű�

name = "test plugin(&T)";

menus = {
    { id = IDM_OPEN, text = "Open(&O)", enable = true },
    {},
    { id = IDM_END, text = "disable item", enable = true }
};

function OnMenuSelected(id)
    if id == IDM_OPEN then
        return "setting";
    elseif id == IDM_END then
    end

    print(info.source .. "\t not impl item = " .. id);
    return nil;
end