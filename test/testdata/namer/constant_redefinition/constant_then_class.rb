# typed: true

R = 5 # error: Cannot initialize the class `R` by constant assignment
class R; end

# The static field always mangles the class definition, so this is not allowed
x = R.new # error: Method `new` does not exist on `Integer`
# this should not resolve as the constant, so this will be an error
puts R + 1
