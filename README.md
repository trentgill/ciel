# Not a thing yet!
This is just a fun little idea I had for a project and might work on from time to time when I want to stretch the programmer muscle. In the end this is probably just a poorly built implementation of C++ with constexpr (a language i senselessly refuse to learn).

While I might end up using this in production, there is no official release planned, and everything here should be read as a bunch of random thoughts from someone who has a very limited experience of the world of embedded development. There's probably some good ideas here, which means they probably already exist in some place that I'm not aware of.

# Ciel platform

Combines C (lang) and Lua (or L for short). Essentially just an alternate input mode for C programming. The Lua frontend is run at compile-time allowing for arbitrary analysis of the files & propogation of constants. Resulting C-code can be dramatically simpler while allowing the source code to be far more readable.

Furthermore, the full Lua environment is available at compile time, so pre-computed LUTs and binary-blobs can be generated directly in the source-code with no runtime-penalty. Code-generation is also supported directly, expecting a list of elements to generate, and a generator function written in pure lua (typically included from another file).

There is a classic problem in C programs where runtime efficiency is improved by using #define compile-time constants. Canonically these are placed in companion header files next to their usage. It is rarely supported (and incurs runtime penalty) to pass these constants to the functions that use them, very often baking hardware-definitions into the source-code in a large number of different places. Ciel avoids this by allowing functions to require constant arguments - the compiler propagates this requirement upward through the call chain - an error is raised if the value is not constant.

The name is at first just a combination of C & L(ua), the idea is that code in the Ciel platform exists up in the sky where everything is possible. When you run the Ciel-compiler, it concretizes absolutely everything it can & is very aggressive about treating everything as a constant where it can. The whole point is to allow easy to read code while allowing for very high performance. The trade-off is that code-size will often be larger than traditional C code with a HAL as we inline basically everything (though "everything" has all decision-trees removed).

Why Lua?:

We could have built Ciel with a different front-end, but Lua fits the bill for so many reasons:
* Lua syntax is largely familiar to C developers (the target language)
* C functions can be written in native lua with only a few special functions (eg. A() for &, and D() for * )
* Ciel builds on Lua's syntactic sugar, and metatables to provide macro-style helpers without having to customize the language & in an ad-hoc manner.
* Lua runs everywhere, can be compiled for heavily constrained platforms, can reasonably be included in the distribution.
* Can self-host test suites & mocks
* Fast JIT compiler available
* Writing algorithms in Lua can be substantially easier than C due to table handling. Full source-analysis typically allows optimization close to, or faster than, hand-written C.
* Writing C inline is possible & quite natural in terms of syntax. Largely unneeded

Usage:

ciel_namespace() function is special. it registers the file with a given namespace so that it can be referred to externally. all declarations before the end of file are inserted into this active namespace when "public". additionally it loads a bunch of global constants for things like "void" as an identifier not a string. this just makes the 

Standards:
* numerical types are entered as a plain number. negative means signed (eg: 8, -8, 16, -32)
* floats are "f32" or "f64"
* other types are as string
* all C-values carry their full type-annotation (and restrictions) until fully flattened. eg: U8(1) returns a "value" table with {"U8", 1}

^^ update ->
eg: U8() is a lua function. when passed a value it casts it to an unsigned 8bit int. when declaring a function, passing the U8 function itself acts as a type declaration.
When creating complex types, the typename (first letter capitalized) can be used as a function to generate that type from some input. this conversion can itself be programmed as a metamethod in the type's table. Any coercion will try to be done at compile time where possible. If you know this needs to happen at runtime, or if you're unsure, treat that metamethod as runtime code (ie. consider if it's a performance bottleneck).

This means that when you make a library that creates an object (a class) when initialized, calling that library itself *is the type coercion function*. other types can be nested, but the top-most type must represent an object of that type.

This kind of thing where Type designators can be used as smart coersions is taken from Carp (where it probably borrowed it from Rust or Haskell). I like that language and it is far closer to being a production ready tool than this.


Potential tools:

* namespace style file-addressing (autogenerated C names)
* analyzer to note when includes are *not* used
* standardized test runner
* best-guess profiling of embedded code
* visualizer for call-chains (esp for aggressive inlining)
