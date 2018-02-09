
name = "study"
date = 180208
me = { name = "usopp", mail = "@qq.com"}

function add (a,b)
    return a+b
end

print("---------------lua begin---------------")

sum = add(12, 1)
print("add sum is ", sum)

local sssss = lua_call_cpp_fn(1, 2, 3)
print("lua_call_cpp_fn sum is ", sssss)

print("---------------lua end---------------")