# typed: true
# compiled: true
# frozen_string_literal: true

dir = File.expand_path(__dir__.to_s)
autoload(:Foo, dir + '/autoload__2')

module Test
  # Uncommenting the following line makes the test pass
  # puts Foo
  @foo = T.let(nil, T.nilable(Foo))
end
