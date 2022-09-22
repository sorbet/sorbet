# typed: true

module R; def self.x; end; end
R = 5 # error: Cannot initialize the module `R` by constant assignment
module R; end
# even though we've reopened the class here, the constant R still
# "wins", because the first definition of the class is what counts

# this should not resolve as a class, so this will be an error
x = R.x # error: Method `x` does not exist on `Integer`
# this should resolve as the constant, so this would be fine
puts R + 1
