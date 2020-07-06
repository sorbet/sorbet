# frozen_string_literal: true
# typed: true
# compiled: true

# This captures a case where `self` wasn't being added to the closure when
# instance variables were used in the context of a ruby block for exception
# handling. Since instance variable usage is turned into a call to
# `sorbet_instanceVariableGet(<self>, @foo)`, self is implicitly required. This
# test shows the problem by having the rescue case trigger instead of calling
# `A#foo` instead.

class A
  def foo
    puts "middle"
  end
end

class B
  def initialize
    @state = A.new
  end

  def test
    puts "before"
    begin
      @state.foo
    rescue => e
      $stdout.puts "Rescue: #{e}"
    end
    puts "after"
  end
end

B.new.test
