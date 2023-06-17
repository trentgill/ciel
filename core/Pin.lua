--- implements the Pin datatype
--- realistically we should just have a single ciel-include (really don't even need to write it)
--- everything in this style is purely compile time, so no need to be pedantic.

local Pin = {}

local Pin_mt = {
    __call = function(self, portpin)
        return {t=Pin, portpin} -- should use a metatable, not the "t" reference
    end
}

return setmetatable(Pin,Pin_mt)
