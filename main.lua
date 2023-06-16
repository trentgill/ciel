--- Ciel
-- Example main file for a Ciel project

local ciel = require "ciel" -- include the base platform
local dout = require "dout"
-- might need to change to ciel.load "lib.debug_pin" to add the types

-- typical to make each entry the precise table that will be passed to the init fn
-- product types (structs) should be placed here directly as it ensures types are accurate
-- eg: hw = {spi_sound = SPI(instance = 1, clock = "C2", mosi = "C3", miso = "C4")}
local hw = {debug_pin = Dout(Pin "B9")} -- usually a separate file via require
    -- ^^ note Pin() is a type-coercing function

function ciel.main() -- TODO argc, argv
    -- init system params

    -- init hw peripherals. object style
    -- this should be auto-generated from the hw table
    -- it's basically already a declaration
    my_dout = dout.init(hw.debug_pin)

    -- but isn't that just the same as having a list here, like this:
    local debug_pin = dout(Pin "B9") -- arg order can be anything if all args are typed (same as a table call)
        -- at least the fn call gives generalized type-checking at top level
    -- what presents a challenge is how to manage sequencing / partial init


    local i = u8(1) -- numbers in C are always type-annotated (auto could be added later)
    while true do
        my_dout:write(i)
        i = i ^ 1 -- wrap this as flip(i) and endo_flip(i) to do it in place
    end
end
