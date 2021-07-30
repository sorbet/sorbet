# typed: true
# frozen_string_literal: true
# compiled: true

class C
  private def some_private(*args)
    puts "I was able to call some_private"
  end

  def call_some_private_for_me
    some_private
    self.some_private
    some_private(*[1,2,3])
    self.some_private(*[1,2,3])
  end
end

begin
  T.unsafe(C.new).some_private
rescue NoMethodError
  puts "Good, I was unable to call some_private"
end

begin
  T.unsafe(C.new).some_private(*[1,2,3])
rescue NoMethodError
  puts "Good, I was unable to call some_private with splat"
end

C.new.call_some_private_for_me
