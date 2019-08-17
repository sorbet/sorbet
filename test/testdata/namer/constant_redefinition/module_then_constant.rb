# typed: true
# disable-fast-path: true

module R; def self.x; end; end
R = 5 # error: Redefining constant `R`

# this should not resolve as a class, so this will be an error
x = R.x # error: Method `x` does not exist on `Integer` component of `Integer(5)`
# this should resolve as the constant, so this would be fine
puts R + 1
