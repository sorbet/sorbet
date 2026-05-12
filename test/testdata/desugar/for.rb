# typed: strict

extend T::Sig

class A
    def self.each(&blk)
  # ^^^^^^^^^^^^^^^^^^^ error: The method `each` does not have a `sig`
        yield 1,2,3,4,5
        yield 6,7,8,9,0
    end
end

class E
    def self.e=(e) # error: does not have a `sig`
        @e = e; # error: The singleton class instance variable `@e` must be declared using `T.let` when specifying `# typed: strict`
    end
    def self.e # error: does not have a `sig`
        @e # error: Use of undeclared variable `@e`
    end
end

class Main
    def self.main # error: does not have a `sig`

        for a in A do
            puts a.inspect
        end
        A.each do |*forTemp|
            a, = *forTemp
            puts a.inspect
        end

        for a,b in A do
            puts a.inspect
            puts b.inspect
        end
        A.each do |*forTemp|
            a,b = *forTemp
            puts a.inspect
            puts b.inspect
        end
        puts "main"
        # You can put all sorts of things on the left
        for @a,@@b,$c,d,E.e in A do
          # ^^ error: The singleton class instance variable `@a` must be declared using `T.let` when specifying `# typed: strict`
          #    ^^^ error: The class variable `@@b` must be declared using `T.let` when specifying `# typed: strict`
            puts @a.inspect
            puts @@b.inspect
            puts $c.inspect
            puts d.inspect
            puts E.e.inspect
        end
        A.each do |*forTemp|
            @a,@@b,$c,d,E.e = *forTemp
          # ^^ error: The singleton class instance variable `@a` must be declared using `T.let` when specifying `# typed: strict`
          #    ^^^ error: The class variable `@@b` must be declared using `T.let` when specifying `# typed: strict`
            puts @a.inspect
            puts @@b.inspect
            puts $c.inspect
            puts d.inspect
            puts E.e.inspect
        end
    end
end
Main.main

@ivar = T.let([:x, 0], [Symbol, Integer])
@@cvar = T.let([:x, 0], [Symbol, Integer])
$gvar = T.let([:x, 0], [Symbol, Integer])

sig { params(arg: [Symbol, Integer]).void }
def call_target=(arg); end

ConstantTarget = T.let([:x, 0], [Symbol, Integer])
ConstantPathTarget = T.let([:x, 0], [Symbol, Integer])

module Nested
  ConstantPathTarget = T.let([:x, 0], [Symbol, Integer])
end

a = T.let([], T::Array[[Symbol, Integer]])

for @ivar in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end


for @@cvar in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end


for $gvar in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end


for self.call_target in [[:a, 1], [:b, 2], [:c, 3]] # error: Assigning a value to `arg` that does not match expected type `[Symbol, Integer]`
  # isPrivateOk should be true
  "loop body"
end

for self::call_target in [[:a, 1], [:b, 2], [:c, 3]] # error: Assigning a value to `arg` that does not match expected type `[Symbol, Integer]`
  # isPrivateOk should be true
  "loop body"
end

for E.e in [] do # isPrivateOk should be false
  "loop body"
end

for ConstantTarget in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end


for ::ConstantPathTarget in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end

for Nested::ConstantPathTarget in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end

for ::Nested::ConstantPathTarget in [[:a, 1], [:b, 2], [:c, 3]] # error: Expected `[Symbol, Integer]` but found `NilClass` for field
  "loop body"
end

for a[123] in [[:a, 1], [:b, 2], [:c, 3]] do # error: Expected `[Symbol, Integer]` but found `NilClass` for argument `arg1`
  "loop body"
end
