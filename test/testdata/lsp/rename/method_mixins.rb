# typed: true
# frozen_string_literal: true

module Base
end

module BaseWithMethod
  include Base
  def foo
#     ^ apply-rename: [B] newName: bar placeholderText: foo
  end
end

module OtherModuleWithMethod
  def foo
#     ^ apply-rename: [D] newName: bar placeholderText: foo
  end
end

class A
  include BaseWithMethod
  def foo
#     ^ apply-rename: [A] newName: bar placeholderText: foo
  end
end

class B
  include BaseWithMethod
end

class C
  include Base
  def foo
#     ^ apply-rename: [C] newName: bar placeholderText: foo
  end
end

class D < C
end

class E
  include BaseWithMethod
  include OtherModuleWithMethod
end

A.new.foo
B.new.foo
D.new.foo
E.new.foo
#     ^ apply-rename: [E] newName: bar placeholderText: foo
