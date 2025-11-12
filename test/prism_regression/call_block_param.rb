# typed: false

foo {} # empty inline block

foo do # empty do-end block
end

foo { "inline block" }

foo do
  "do-end block"
end

foo { |positional, kwarg:, &block| "inline block with params" }

foo do |positional, kwarg:, &block|
  "inline block with params"
end

foo do |positional, (multi, target)|
  "block with multi-target node in parameter list"
end

# block with parameter `bar` and block locals `baz` and `qux`
foo { |bar; baz, qux| }

foo do |positional, (multi, target); baz, qux|
  "block with multi-target node in parameter list and block locals"
end

foo { |bar; baz, qux| }

foo(&FORWARDED_BLOCK)
foo &FORWARDED_BLOCK

foo(&) # Anonymous block pass argument
# foo &  # Not valid syntax
#      ^ unexpected end-of-input; expected an expression after the operator

foo&.bar {}

foo do |(*args)|
  "block with multi-target rest args"
end

foo do |*args|
  "block with rest args"
end

def_delegators :foo, local => :thing
