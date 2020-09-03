# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def foo; end
end

class B; end

begin
  A.instance_method(:foo).bind(B.new).call
rescue TypeError => exn
  p exn.message
end
