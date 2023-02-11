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
