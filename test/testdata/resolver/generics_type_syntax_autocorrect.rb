# typed: true
extend T::Sig

class Example
  extend T::Generic

  Elem = type_member
end

class Opus::Another
  extend T::Generic

  Elem = type_member
end

module T
  # There's a method called `Kernel#Hash` that Sorbet thinks is being called,
  # which has one argument. When given two, Sorbet thinks it's smart enough to
  # delete the second argument for you. Technically that conflicts with the
  # autocorrect below.
  #
  # But in practice, you would either use IDE code actions to pick the one you
  # want, or use the --isolate-error-code flag to apply from the command line,
  # so let's define this to make it easier to test via autocorrect snapshots.
  def self.Hash(x, y); end
end


sig { params(x: T::Array(String)).void }
#               ^^^^^^^^^^^^^^^^ error: Did you mean to use square brackets: `T::Array[String]`
def test1(x)
end

sig { params(x: Example(String)).void }
#               ^^^^^^^^^^^^^^^ error: Did you mean to use square brackets: `Example[String]`
#               ^^^^^^^ error: does not exist
def test2(x)
end

sig { params(x: Opus::Another(String)).void }
#               ^^^^^^^^^^^^^^^^^^^^^ error: Did you mean to use square brackets: `Opus::Another[String]`
#                     ^^^^^^^ error: does not exist
def test3(x)
end

sig { params(x: T::Hash(Symbol, Integer)).void }
#               ^^^^^^^^^^^^^^^^^^^^^^^^ error: Did you mean to use square brackets: `T::Hash[Symbol, Integer]`
def test4(x)
end

