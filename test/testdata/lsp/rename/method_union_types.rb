# typed: true
# frozen_string_literal: true

class C1
  def m1
#      ^ apply-rename: [A] newName: m2 placeholderText: m1
  end
end

class C2
  def m1
#      ^ apply-rename: [B] newName: m2 placeholderText: m1
  end
end

class C11 < C1
  def m1
#      ^ apply-rename: [D] newName: m2 placeholderText: m1
  end
end

class C3
  def m1
#      ^ apply-rename: [E] newName: m2 placeholderText: m1
  end
end

class CallerClass
  extend T::Sig

  sig { params(c: T.any(C1, C2)).void }
  def caller(c)
    c.m1
#      ^ apply-rename: [C] newName: m2 placeholderText: m1
  end

  sig { params(c: T.any(C11, C3)).void }
  def caller2(c)
    c.m1
  end
end
