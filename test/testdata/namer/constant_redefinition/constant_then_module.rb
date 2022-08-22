# typed: true

R = 5
module R; def self.x; end; end # error: Redefining constant `R` as a class or module

# this should resolve as a class, so this would not be an error
x = R.x
# this should not resolve as the constant, so this will be an error
puts R + 1 # error: Method `+` does not exist on `T.class_of(R)`
