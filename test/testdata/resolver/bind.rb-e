# typed: true
class Foo
  extend T::Sig

  sig {bind(Integer).returns(Integer)}
  def int
    self + 2
  end

  sig {bind(Integer).bind(Symbol).returns(Integer)} # error: Malformed `bind`: Multiple calls to `.bind`
  def double_bind; 1; end

  sig {bind(Bar).returns(Integer)}
  def bar; 1; end

  sig {bind(T.any(Integer, String)).void} # error: Malformed `bind`: Can only bind to simple class names
  def too_complex; end
end

class Bar
end
