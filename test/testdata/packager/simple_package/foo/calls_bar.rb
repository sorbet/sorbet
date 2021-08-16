# frozen_string_literal: true
# typed: strict

module Project::Foo::CallsBar
  extend T::Sig

  sig {returns(Project::Bar::Bar)}
  def self.build_bar
    # Construct an imported class.
    Project::Bar::Bar.new(10)
  end
end
