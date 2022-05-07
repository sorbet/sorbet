# frozen_string_literal: true
# typed: strict

class Project::Foo::Foo
  puts Project::Bar::This
  #    ^^^^^^^^^^^^^^^^^^ error: `Project::Bar::This` resolves but its package is not imported
end
