# frozen_string_literal: true
# typed: true
# compiled: true


module Flag
  extend T::Helpers
  extend T::Sig

  T_ATTRIBUTES = T.type_alias {T::Hash[Symbol, T.untyped]}
  EMPTY_ATTRIBUTES = T.let({}.freeze, T_ATTRIBUTES)

  sig {params(flag_name: String, attributes: T_ATTRIBUTES).returns(T::Boolean)}
  def self.active?(flag_name, attributes=EMPTY_ATTRIBUTES)
    p flag_name
    p attributes
    return true
  end
end

3.times do
  p Flag.active?("foo")
  p Flag.active?("bar", {:attr => 1})
end
