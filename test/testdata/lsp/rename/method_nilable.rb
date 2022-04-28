# typed: true
# frozen_string_literal: true

class C1
  def m1
#      ^ apply-rename: [A] newName: m2 placeholderText: m1
  end
end

class CallerClass
  extend T::Sig

  sig { params(c: T.nilable(C1)).void }
  def caller(c)
    c&.m1
#      ^ apply-rename: [B] newName: m2 placeholderText: m1
  end
end
