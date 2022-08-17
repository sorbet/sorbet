# typed: true

module R; def self.x; end; end
R = 5 # error: Redefining constant `R` as a static field

# this should not resolve as a class, so this will be an error
x = R.x # error: Method `x` does not exist on `Integer`
# this should resolve as the constant, so this would be fine
puts R + 1
