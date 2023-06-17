-- these will eventually be automatic, or at least point to ciel versions
hash_include( "<stm32f7xx.h>"
            , "<stm32f7xx_hal.h>"
            , "<stm32f7xx_hal_rcc.h>"
            , "<stm32f7xx_hal_cortex.h>"
            , "syscalls.c"
            , "system.h"
            , "oldlib/dout.h")

ciel_include{"Pin", "Dout", "oldlib"}

-- static declaration mimic-ing C version. No need to separate this as it *must* happen at compile time
-- the return value of a type constructor should never make it into runtime code
-- instead, it's all flattened down to simple data constants throughout the program.
local led = Dout(Pin "B7") -- data representation of a Dout

function main() -- TODO argc, argv
    -- system_init() -- opaque whether this is Lua or C code (on purpose. should feel smooth)
    in_c(system_init) -- for now we do explicit syntax. args are passed as sequential params

    -- led:init() -- explicit initialization of the hardware

    -- local last = u32(0)
    -- while true do
    --     if last ~= HAL_GetTick() then
    --         last = HAL_GetTick()
    --         if last % 66 == 0 then
    --             led:flip()
    --         end
    --     end
    -- end
end

main{ in_c(system_init)
    , }

main{ system_init
    , let("LD2")

-- fennel version
(defn main []
    (system_init)
    (let [LD2 (dout_init :B7)]
        (dout_set LD2, 1)
        (var last 0)
            (while true
                (let [tick (HAL_GetTick)]
                    (when (not= last tick)
                        (set last tick)
                        (when (= 0 (% last 66))
                            (dout_flip LD2)))))))

-------------------------------------------------------------------------
-- above should generate the below (for starters)
--[[

static Dout* LD2;

int main(void)
{
    system_init();

    LD2 = dout_init("B7");
    
    dout_set(LD2, 1);

    uint32_t lasttick = 0; // throttle
    while(1){
        // 1ms throttled sub-loop
        if( lasttick != HAL_GetTick() ){
            lasttick = HAL_GetTick();
            if( lasttick % 66 == 0 )
                dout_flip(LD2);
        }
    }
    return 0;
}

]]--
