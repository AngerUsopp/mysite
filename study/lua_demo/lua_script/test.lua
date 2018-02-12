local lp = require "lua_proxy"

print(os.date());
print(lp:average(1,2,3,4, "asd"));
print("lua print_str " .. lp:print_str("lua_proxy"));

proxy_obj = lp:CreateCLuaProxy();
print(proxy_obj:SayHello());
--proxy_obj = nil;


--name = "study"
--date = 180208
--me = { name = "usopp", mail = "@qq.com"}
--
--function add (a,b)
--    print("lua add");
--    return a+b
--end
--
--print("---------------lua begin---------------")
--
--sum = add(12, 1)
--print("add sum is ", sum)
--
--local sssss = lua_call_cpp_fn(1, 2, 3)
--print("lua_call_cpp_fn sum is ", sssss)
--
--print("---------------lua end---------------")
