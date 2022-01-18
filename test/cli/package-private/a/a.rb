# typed: strict

class A
  extend T::Sig

  sig {packageprivate.returns(String)}
  def self.foo
    '1'
  end

  sig {packageprivate.returns(String)}
  def foo
    '1'
  end

  sig {void}
  def test
    self.class.foo
    foo
  end
end
