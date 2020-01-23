# frozen_string_literal: true
# typed: true
# compiled: true

at_exit {
  foo
}

def foo
  puts "foo"
end
