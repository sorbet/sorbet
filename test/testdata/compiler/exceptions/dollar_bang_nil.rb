# frozen_string_literal: true
# typed: true
# compiled: true

class FooError < RuntimeError; end

begin
  raise FooError.new('boo')
rescue
  p $!
end
