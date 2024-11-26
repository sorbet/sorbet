# typed: true

class A < T::Struct
  prop :'with-hyphen', Integer
  #      ^^^^^^^^^^^ error: Bad prop name "with-hyphen"
  prop :$dollar, Integer
  #     ^^^^^^^ error: Bad prop name "$dollar"
  prop :"", Integer
  #     error: Bad prop name ""
  prop "not_a_symbol", String
  #    ^^^^^^^^^^^^^^ error: Expected `Symbol` but found `String("not_a_symbol")` for argument `name`
end

class B
  attr_reader :'with-hyphen'
  #           ^^^^^^^^^^^^^^ error: Bad attribute name "with-hyphen"
  attr_reader :$dollar
  #           ^^^^^^^^ error: Bad attribute name "$dollar"
  attr_reader :""
  #           ^^^ error: Bad attribute name ""
end
