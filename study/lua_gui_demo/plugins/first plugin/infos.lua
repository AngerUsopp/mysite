--引用另一个lua文件的方式dofile、require
--[[以dofile方式“引用”需要得到目标文件的绝对路径，并且会执行一遍脚本，为了避免执行不必要的操作，
则目标文件里面应该仅仅只是定义变量/函数，不应有任何函数调用操作
]]--
local info = debug.getinfo(1, "S"); -- 第二个参数 "S" 表示仅返回 source,short_src等字段， 其他还可以 "n", "f", "I", "L"等 返回不同的字段信息 
local path = info.source;
path = string.sub(path, 2, -1) -- 去掉开头的"@"    
path = string.match(path, "^.*\\") -- 捕获最后一个 "/" 之前的部分 就是我们最终要的目录部分
dofile(path .. "ids.lua");  -- 虽然可以“引用”，但需要加载并执行整份脚本

--[[以require方式能真正引用文件，无需执行一遍。但根据lua的检索顺序，相应lua文件必须放在宿主程序路径下，
如果文件本身在一个多层级的目录中则整个目录必须在宿主程序目录下，并且lua文件内部需要确切知道自己与宿主
程序的完整相对路径，今后若是稍微改动一下目录结构，则必须一一修改其内部全部的lua文件代码
]]--
--require("plugins\\test plugin\\ids");   -- 直接引用lua文件需要确切知道/获取目标文件与宿主exe的相对路径，不会执行脚本

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