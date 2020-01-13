# typed: true

class Test
  extend T::Sig

  sig {returns(T.nilable(String))}
  attr_accessor :a

  sig {returns(T.nilable(Integer))}
  attr_accessor :b, :c

  sig {returns(String)}
  attr_reader :d

  sig {returns(String)}
  attr_reader :e, :f

  sig {params(g: String).returns(String)}
  attr_writer :g

  sig {params(h: String).returns(String)} # error: Malformed `params`: Multiple calls to `.params`
  attr_writer :h, :i
end
