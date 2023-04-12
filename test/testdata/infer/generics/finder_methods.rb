# typed: true

module FinderMethods
  extend T::Sig
  extend T::Generic
  abstract!

  has_attached_class!

  sig {abstract.returns(T.attached_class)}
  def new; end

  sig {params(id: String).returns(T.attached_class)}
  def find(id)
    self.new
  end
end

class Charge
  extend T::Sig
  extend FinderMethods

  sig {returns(Integer)}
  def foo; 0; end
end

x = Charge.find('ch_123')
T.reveal_type(x.foo) # error: `Integer`
