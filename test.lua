x = {test = "bar", foo = "no", food = "yes"}

y = require("test2")

for k,v in pairs(x) do
	print(k, "=>", v)
end

print("GOT", y, y[10])
