# frozen_string_literal: true
# typed: strict

class Project::Foo::Foo
  puts Project::Bar::This
     # ^^^^^^^^^^^^^^^^^^ error: No import provides `Project::Bar::This`
end
