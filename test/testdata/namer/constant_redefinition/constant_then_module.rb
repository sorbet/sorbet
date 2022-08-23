# typed: true

R = 5 # error: Cannot initialize the module `R` by constant assignment
module R; def self.x; end; end

# this should resolve as a class, so this would not be an error
x = R.x # error: Method `x` does not exist on `Integer`
# this should not resolve as the constant, so this will be an error
puts R + 1
