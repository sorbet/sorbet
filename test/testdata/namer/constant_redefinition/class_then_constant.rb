# typed: true

class R; end
R = 5 # error: Cannot initialize the class `R` by constant assignment

# this should not resolve as a class, so this will be an error
x = R.new # error: Method `new` does not exist on `Integer`
# this should resolve as the constant, so this would be fine
puts R + 1
