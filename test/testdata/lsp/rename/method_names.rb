# typed: strict
# frozen_string_literal: true

class C1
  extend T::Sig

  sig { returns(C1) }
  def m1
#     ^ apply-rename: [A] newName: m2 placeholderText: m1
    self
  end

  sig { returns(String) }
  def x
    "hello, world"
  end
end

class C2
  extend T::Sig

  # this one isn't being renamed
  sig { returns(C1) }
  def m1
    C1.new
  end
end

c2 = C2.new
# only the second m1 should change
puts c2.m1.m1.x
puts c2.m1().m1.x
puts c2.m1().m1().x

c1 = C1.new
# both first and second m1 should change
puts c1.m1.m1.x
