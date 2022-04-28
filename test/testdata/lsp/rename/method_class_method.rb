# typed: true
# frozen_string_literal: true

class Foo
  def self.foo
#          ^ apply-rename: [A] newName: bar placeholderText: foo
  end

  def foo
    # this is a separate method
  end
end

Foo.foo
Foo.new.foo
