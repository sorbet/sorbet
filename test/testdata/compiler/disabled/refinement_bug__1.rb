# typed: false
# frozen_string_literal: true
# compiled: false

require_relative './refinement_bug__2'

class Object
  def blank?
    !self
  end
end

A.foo

module Thinger
  refine Object do
    def blank?
      !self
    end
  end
end

A.foo
