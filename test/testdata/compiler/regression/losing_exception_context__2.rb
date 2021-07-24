# frozen_string_literal: true
# typed: true
# compiled: false

module Test2

  def self.foo
    puts "hi"
    raise 'bar'
  rescue
  end

end
