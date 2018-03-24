module T
  sig(x: T.untyped).returns(T.untyped)
  def self.unsafe(x); end

  # Once we get generic methods, it should return T.nilable(<type>)
  sig(
    obj: T.untyped,
    type: Class
  ).returns(T.untyped)
  def self.dynamic_cast(obj, type); end
end

module T::Helpers
end
module T::Generic
  include T::Helpers
end
