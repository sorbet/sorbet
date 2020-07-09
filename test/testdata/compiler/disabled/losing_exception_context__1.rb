# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './losing_exception_context__2.rb'

begin
  raise 'foo'
rescue
  puts "$! = #{$!.inspect}"
  Test2.foo
  puts "$! = #{$!.inspect}"
end
