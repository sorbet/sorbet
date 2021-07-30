# frozen_string_literal: true
# typed: true
# compiled: true

require_relative './losing_exception_context_ensure__2.rb'

begin
  begin
    raise 'foo'
  ensure
    puts "$! = #{$!.inspect}"
    Test2.foo
    puts "$! = #{$!.inspect}"
  end
rescue
end
