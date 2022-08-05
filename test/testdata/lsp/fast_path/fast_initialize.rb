# typed: true

# There was once a time where we hard-coded certain method names to always take
# the slow path, because of some limitations in how Sorbet's fast path handled
# updates that changed few files but had many downstream files that needed to
# be retypechecked. `initialize` was one of those names.

class A_01
  extend T::Sig
  sig {params(x: Integer).void}
  def initialize(x)
    T.reveal_type(x) # error: `Integer`
  end
end
