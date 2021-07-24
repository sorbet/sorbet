# frozen_string_literal: true
# typed: true
# compiled: true

def f
  yield
end

def g
  begin
    raise "oops"
  rescue
    puts "exception"
  ensure
    f { puts "hello" }
  end
end

g
