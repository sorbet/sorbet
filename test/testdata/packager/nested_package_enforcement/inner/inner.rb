# frozen_string_literal: true
# typed: true

module Outer
  def self.bad
# ^^^^^^^^^^^^ error: Class or method behavior may not be defined outside of the enclosing package namespace `Outer::Inner`
  end


  module Inner
    module Mixin; end
    ABC = 'abc'
  end

  include Inner::Mixin
# ^^^^^^^^^^^^^^^^^^^^ error: Class or method behavior may not be defined outside of the enclosing package namespace `Outer::Inner`
end
