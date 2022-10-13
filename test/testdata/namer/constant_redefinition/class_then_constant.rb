# typed: true

class R; end
R = 5 # error: Cannot initialize the class `R` by constant assignment

# The static field never mangles the class definition, so this is allowed
x = R.new
# this resolves as the class, so this will be an error
puts R + 1 # error: Method `+` does not exist on `T.class_of(R)`
