hash_include("<stm32f7xx.h>") -- supports n-args in order -->

local dout = ciel_namespace()
-- namespace function returns a table with a single entry called "ciel" where you
-- store *all* the compile time information about the state of the machine.
-- this data never makes it on-chip, but it allows direct injection of port-accesses
-- while maintaining the feeling of object-permanence in the code.

-- public interface (this is the 'header' file)
-- methods
-- types
-- dout.__call -- use this to provide either a type coersion or constructor

-- functions in the parent table are *public* as if they're exposed in a header file
-- local functions are private declarations

-- type constructor
local function Speed(str)
    -- TODO convert string of speed into bitmask representation per datasheet
    return 1
end

local function dout_type(pin)
    return ciel.newtype("Dout", {gpio = pin.gpio -- this is a ref to a table with metamethod lookup
                                ,pin_number = pin.number
                                ,speed = Speed("high")
                                ,enabled = false})
end

-- constructor
function dout.init(pin) -- pin is a complex type constructed by the Pin function (includes string, gpio, number)
    ciel.check_args(t.pin ~= nil) -- check for valid args (at compile time when possible)

    local self = dout_type(Pin)
    rcc.clock_enable(self.gpio)

    self.gpio.config{pin = self.pin_number
                    ,speed = self.speed
                    ,otype = "output_pp"
                    ,pull = "none"
                    ,mode = "output"}
    self.enabled = true
    return self
end

function dout:write(state)
    -- note re: gpiox->ODR becoming self.gpio.ODR
    -- it becomes opaque whether we're extracting a value (.), or deref-ing to a value (->)
    -- perhaps this is ok? as long as we can solve this automatically with more complex types
    -- can we get similar behaviour with a : method lookup?
    WRITE_REG(self.gpio.ODR, state << self.pin_number)
end

function dout:set()
    WRITE_REG(self.gpio.BSRR, 1 << self.pin_number)
end

function dout:reset()
    WRITE_REG(self.gpio.BSRR, 1 << (self.pin_number + 16))
end

-- if you need to do an atomic write to multiple pins on the gpio just access the register directly:
-- eg: my_dout.gpio.ODR = 0b10010110110
-- or wrap in WRITE_REG for clarity
