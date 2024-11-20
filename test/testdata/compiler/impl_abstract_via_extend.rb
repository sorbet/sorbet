# frozen_string_literal: true
# typed: strict
# compiled: true

module IFoo
  extend T::Helpers
  extend T::Sig
  abstract!

  sig {abstract.returns(Integer)}
  def foo; end
end


require_relative './impl_abstract_via_extend__1'
