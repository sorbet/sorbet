# typed: true

module Consumer
  BAD = T.let(1, Integer) # error: File belongs to package `Consumer::Nested` but defines a constant that does not match this namespace

  BAD_NESTED = class ::Unrelated # error: File belongs to package `Consumer::Nested` but defines a constant that does not match this namespace
  #            ^^^^^^^^^^^^^^^^^ error: Defining a root-scoped constant requires this package to be marked `prelude!`
    def self.inside_rhs
    end
  end

  def self.after_assignment # error: This file must only define behavior in enclosing package `Consumer::Nested`
  end
end
