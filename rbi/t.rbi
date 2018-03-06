module T
  sig(x: T.untyped).returns(T.untyped)
  def self.unsafe(x); end
end

module T::Helpers
end
module T::Generic
  include T::Helpers
end
