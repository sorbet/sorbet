# frozen_string_literal: true
# typed: true
# compiled: true

class C
  def f(&blk)
    yield "hi"
    yield "hi"
  end
end

class D < C
  def f(&blk)
    puts "in D"
    super
  end
end

D.new.f { |x| puts "in block: #{x}" }
