# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def self.test(b)
    begin
      if b
        raise "err"
      end
      10
    rescue
      20
    else
      30
    ensure
      40
    end
  end
end

puts(A.test true)
