--- sai pll calculator

BASE_FREQ = 1 -- in Megahertz. Might need to change depending on crystal?

-- N can multiply by 50~432
-- Q can divide by 2~15
-- DivQ can divide further by 1~32
function param_search(target)
    -- multipliers
    local t = {}
    for i=50,432 do
        table.insert(t, {i, SAIN=i})
    end

    -- division
    local t2 = {}
    for i=2,15 do
        for k,v in pairs(t) do
            table.insert(t2, {v[1]/i, SAIN=v.SAIN, SAIQ=i})
        end
    end

    -- second division
    local t3 = {}
    for i=1,32 do
        for k,v in pairs(t2) do
            table.insert(t3, {v[1]/i, SAIN=v.SAIN, SAIQ=v.SAIQ, DivQ=i})
        end
    end

    -- sort
    table.sort(t3, function(k1, k2) return k1[1] < k2[1] end)

    -- prune
    local t4 = {}
    local previous = 0
    for k,v in ipairs(t3) do
        if v[1] ~= previous then
            table.insert(t4, v)
            previous = v[1]
        end
    end

    -- match
    local upper, lower = 0, 0
    local exact = false
    for k,v in ipairs(t4) do
        if v[1] == target then
            exact = k
            break
        elseif v[1] < target then
            lower = k
        else -- v[1] is greater than target
            upper = k
            break
        end
    end

    -- find error
    local match_key = {}
    local error_percent = 0
    if not exact then
        local lower_v, upper_v = t4[lower][1], t4[upper][1]
        local lower_error = (target - lower_v) / lower_v
        local upper_error = (target - upper_v) / upper_v 
        if math.abs(upper_error) < math.abs(lower_error) then
            error_percent = upper_error
            match_key = upper
        else
            error_percent = lower_error
            match_key = lower
        end
    else
        match_key = exact
        error_percent = 0
    end

    local mtab = t4[match_key]

    -- return matched, error_percent, mul, div1, div2
    return mtab[1], error_percent, mtab.SAIN, mtab.SAIQ, mtab.DivQ 
end



--- usage:
-- enter your desired frequency, and you'll be shown the
-- closest match to your desired frequency with error %
-- and the mul/div/div options

local desired_freq = tonumber(arg[1])
print(string.format("desired: %gMHz\n"
   .. "closest: %gMHz\n"
   .. "error: %g%%\n"
   .. "SAIN: %d\n"
   .. "SAIQ: %d\n"
   .. "DivQ: %d\n", desired_freq, param_search(desired_freq)))
