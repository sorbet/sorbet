# frozen_string_literal: true
# typed: true
# compiled: true

module Log
  def self.error(*args); end
end

class A
  def test
    puts "Populate flags!"
  end
end

module B
  extend T::Sig

  def self.init
    @state = A.new
  end

  sig {void}
  def self.test
    puts "before"
    begin
      @state.test
    rescue => e
      Log.error('Error loading updated feature flags', e)
    end
    puts "after"
  end
end

B.init
B.test
