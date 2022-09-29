# frozen_string_literal: true
# typed: strict

class Project::Bar::Bar
  extend T::Sig
  sig {params(value: Integer).void}
  def initialize(value)
    @value = T.let(value, Integer)
  end

  puts(Project::Bar::Bar)
  #                  ^ hover: T.class_of(Project::Bar::Bar)
  #                  _________________________________________
  puts(Project::Foo::Foo)
  #                  ^ hover: T.class_of(Project::Foo::Foo)
  #                  _________________________________________

  # __________
end
