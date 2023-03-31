# typed: false

# Code like this comes up from time to time when when running Sorbet over a
# random Ruby gem, e.g. where Sorbet can't see that DoesNotExist is a class
# that exists in some _other_ gem, not the one we're currently running over.
#
# The only behavior we care about here is that Sorbet doesn't crash

class A < DoesNotExist # error: The super class `DoesNotExist` of `A` does not derive from `Class`
  ::DoesNotExist::X = self
end
