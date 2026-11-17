# frozen_string_literal: true

# typed: strict

class Project::Bar::Bar
  extend T::Sig

  sig { void }
  def bar1
    Project::Foo::D.d
    Project::FooBar::Z.z
  end

  sig { void }
  def bar2
    Project::Baz::Package::E.e
  end
end
