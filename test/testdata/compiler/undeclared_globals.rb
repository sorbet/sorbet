# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def set_a
    $a = 1
  end

  def set_b
    $b = 2
  end

  def debug
    p $a
    p $b
  end
end

a = A.new
a.debug
a.set_a
a.debug
a.set_b
a.debug
