# frozen_string_literal: true
# typed: true

module T::Props::HashAccessible
  extend T::Sig

  sig { params(property_name: T.any(String, Symbol)).returns(T.untyped) }
  def [](property_name)
    raise NameError.new("no member #{property_name} in T::Struct") unless respond_to?(property_name)

    public_send(property_name)
  end
end
