# typed: true

module R; def self.x; end; end
R = 5 # error: Cannot initialize the module `R` by constant assignment

# The static field never mangles the module definition, so this is allowed
x = R.x
# this resolves as the module, so this will be an error
puts R + 1 # error: Method `+` does not exist on `T.class_of(R)`
