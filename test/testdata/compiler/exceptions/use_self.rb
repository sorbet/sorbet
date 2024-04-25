# frozen_string_literal: true
# typed: true
# compiled: true

class A
  def test
    raise "foo"
  end

  def run
    begin
      self.test
    rescue => e
      puts e.message
    end
  end
end

A.new.run
