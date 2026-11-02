# frozen_string_literal: true
# typed: strict

module Test::Foo::Bar
  # Ensure that bad exports don't cause errors in test namespace
  Foo::Bar::Exists
end
