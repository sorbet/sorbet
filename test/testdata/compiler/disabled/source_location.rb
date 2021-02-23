# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def foo; end
end

p A.instance_method(:foo).source_location
