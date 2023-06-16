
function ciel.main() -- TODO argc, argv
    system_init() -- opaque whether this is Lua or C code (on purpose. should feel smooth)

    local led = Dout(Pin "B7") -- data representation of a Dout
    led:init() -- explicit initialization of the hardware

    local last = u32(0)
    while true do
        if last ~= HAL_GetTick() then
            last = HAL_GetTick()
            if last % 66 == 0 then
                led:flip()
            end
        end
    end
end
