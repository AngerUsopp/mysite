local lp = require "lua_proxy";

print("---------------lua begin---------------");

print(os.date());
print("lua average " .. lp:average(1,2,3,4, "asd"));
print("lua print_str " .. lp:print_str("lua_proxy"));

proxy_obj = lp:CreateCLuaProxy();
print(proxy_obj);
proxy_obj:SayHello();
--proxy_obj = nil;

proxy_obj1 = lp:CreateCLuaProxy();
print(proxy_obj1);
proxy_obj1.SayHello(proxy_obj1);
--proxy_obj1 = nil;

--collectgarbage("collect");--proxy_obj = nil之后不会立即gc，调用collectgarbage立即gc

g_member = "study";
g_table = { name = "usopp", mail = "xxx@qq.com"}
g_vector = { 1, 1.5, "string" };

function add (a,b)    
    print("add p_a is ", a);
    print("add p_b is ", b);
    return a+b;
end

local sum = add(11, 22);
print("add sum is ", sum);

if lua_call_cpp_fn then
    local ret0, ret1 = lua_call_cpp_fn(1, 2, 3);
    print("lua_call_cpp_fn sum is ", ret0, ret1);
end

if g_vector then
    print("g_vector members");
    for i = 1, #g_vector do
        print("v[" , i, "]", g_vector[i]);
    end
end

function check_cpp_global()
    --数组遍历
    if cpp_vector then
        print("cpp_vector members begin");
        for i = 1, #cpp_vector do
            print("v[" , i, "]", cpp_vector[i]);
        end
        print("cpp_vector members end");
    end

    --map遍历
    if cpp_map then
        print("cpp_map members begin");
        for k,v in pairs(cpp_map) do 
            print("k = ",k," v = ",v)
        end
        print("cpp_map members end");
    end
end

print("---------------lua end---------------");
