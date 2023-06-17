--- ciel compiler
-- takes in a main.lua file and generates the output C
-- right now it's plain lua->C conversion
-- later it will be optimized in an aggressive, constant-forward approach


-------------------------------------------------------
-- global state to hold the project
-- this object will get *big* you'll want a system with lots of RAM for big projects

_INFO_ = "\n// Ciel v0.0\n//  this C code was generated from a lua file!\n"
_STATE_ = {_INFO_} -- basically a table of strings that get table.concat'd at the very end

-------------------------------------------------------
-- global functions for use in a ciel project

function hash_include(a,...)
    -- recursive just for fun
    -- should make it take a table instead
    if a then
        local fstr = "#include %s"
        if a:sub(1,1) ~= '<' then -- global include
            fstr = "#include \"%s\""
        end
        table.insert(_STATE_, string.format(fstr, a))
        hash_include(...) -- tail call
    end
end

function ciel_include(t)
    for _,v in pairs(t) do
        _G[v] = require("core." .. v)
    end
end

function in_c(fn,...)
    table.insert
end


-------------------------------------------------------
-- main compiler

function ciel_compile(in_file)
    -- print(dofile(in_file))
    local main = require(in_file)
    return table.concat(_STATE_,"\n")
end


-------------------------------------------------------
--- process input directory & generate file
-- if no destination, then prints to console

local in_file = arg[1]
local out_file = arg[2]

do
    local compiled = ciel_compile(in_file)
    if arg[2] then -- write to disk
        local o = io.open(out_file, 'w')
        o:write(compiled)
        o:close()
    else -- write to stdio
        print(compiled)
    end
end

-- example usage:
-- lua compiler.lua example.main build/main.c
-- ^^ note the example.main == example/main.lua (lua's require syntax)
