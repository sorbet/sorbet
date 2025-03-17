# typed: strict
# enable-experimental-rbs-signatures: true

Class.new do
  #: -> String
  def foo
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

Class.new do
  #: -> String
  def bar
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  puts "bar"

  #: -> String
  def baz
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  # Note that attr accessors can't have a sig (either RBS or Sorbet sig block) when found in a block.
  attr_reader :qux1
  attr_reader :qux2
end
