# frozen_string_literal: true
# typed: strict

class Project::Foo::Foo
  puts Project::Bar::This
  #    ^^^^^^^^^^^^^^^^^^ error: `Project::Bar::This` is not imported
end
