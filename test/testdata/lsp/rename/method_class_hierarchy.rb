# typed: true
# frozen_string_literal: true

class Base
  def foo
  end
end

class A < Base
  def foo
#     ^ apply-rename: [A] newName: bar
  end
end

class B < Base
  def foo
  end
end

A.new.foo
