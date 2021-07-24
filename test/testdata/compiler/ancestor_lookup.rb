# frozen_string_literal: true
# typed: true
# compiled: true
module M
  def foo
    1
  end
end

class A
  include M
end

puts A.new.foo
