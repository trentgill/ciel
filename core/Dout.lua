--- implements the Dout datatype

-- local Pin = require "Pin"

local Dout = {}


local Dout_mt = {
    __call = function(self, pin) -- type constructor
        return setmetatable({pin}, Dout)
    end
}

return setmetatable(Dout,Dout_mt)
