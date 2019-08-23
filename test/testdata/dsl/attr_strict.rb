# typed: strict

class Test
  extend T::Sig

  sig {returns(String)}
  attr_accessor :non_nil_accessor # error-with-dupes: Use of undeclared variable `@non_nil_accessor`
  sig {returns(T.nilable(String))}
  attr_accessor :nilable_accessor

  sig {returns(String)}
  attr_reader :non_nil_reader # error: Use of undeclared variable `@non_nil_reader`
  sig {returns(T.nilable(String))}
  attr_reader :nilable_reader # error: Use of undeclared variable `@nilable_reader`

  sig {params(non_nil_writer: String).returns(String)}
  attr_writer :non_nil_writer # error: Use of undeclared variable `@non_nil_writer`
  sig {params(nilable_writer: String).returns(T.nilable(String))}
  attr_writer :nilable_writer
end
