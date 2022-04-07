# frozen_string_literal: true
# typed: strict

class Project::Foo::Foo
  extend T::Sig
  sig {params(value: Integer).void}
  def initialize(value)
    @value = T.let(value, Integer)
  end
end
