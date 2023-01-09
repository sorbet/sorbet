# frozen_string_literal: true
# typed: true

module Outer
  def self.bad
# ^^^^^^^^^^^^ error: This file must only define behavior in enclosing package `Outer::Inner`
  end


  module Inner
    module Mixin; end
    ABC = 'abc'
  end

  include Inner::Mixin
# ^^^^^^^^^^^^^^^^^^^^ error: This file must only define behavior in enclosing package `Outer::Inner`
end
