# frozen_string_literal: true
# typed: strict
# compiled: true

# It's kind of tricky to test the runtime behavior of T.absurd, because we have
# so many static checks in place to ensure that all places where `T.absurd`
# would have raised are actually static errors.
#
# One place where we *don't* currently check this is element types of
# collections (we used to do these checks in sorbet-runtime but they were too
# slow, and the compiler never compiled these element type checks).
# We're abusing that fact to sneak a value through the type checker that
# doesn't actually have the asserted type, so that the absurd fails at runtime.

class Main
  extend T::Sig

  # Drop the .checked(:never) once we change sorbet-runtime to not do element type tests.
  sig {params(xs: T::Array[Integer]).void.checked(:never)}
  def self.main(xs)
    x = xs[0]
    case x
    when NilClass then x
    when Integer then x
    else
      T.absurd(x)
    end
  end
end

begin
  Main.main(T.unsafe(['nope']))
rescue TypeError => exn
  p exn.message
end
