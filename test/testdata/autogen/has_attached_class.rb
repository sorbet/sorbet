# typed: true

module HasAttachedClassExample
  extend T::Sig
  extend T::Generic

  abstract!
  has_attached_class!

  sig {abstract.returns(T.attached_class)}
  def example; end
end
