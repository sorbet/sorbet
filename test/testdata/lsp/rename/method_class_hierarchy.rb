# typed: true
# frozen_string_literal: true

class Base
  # does not contain method foo
end

class BaseWithMethod < Base
  def foo
  end
end

class A < BaseWithMethod
  def foo
#     ^ apply-rename: [A] newName: bar placeholderText: foo
  end
end

class B < BaseWithMethod
  def foo
  end
end

class C < Base
  def foo
    # this one should not be renamed!
  end
end

A.new.foo
