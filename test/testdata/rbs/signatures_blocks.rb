# typed: strict
# enable-experimental-rbs-comments: true

Class.new do
  #: -> String
  def def1
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

Class.new do
  #: -> String
  def def2
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  puts "bar"

  #: -> String
  def def3
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  # Note that attr accessors can't have a sig (either RBS or Sorbet sig block) when found in a block.
  attr_reader :attr1
  attr_reader :attr2
end

Const1 = Class.new do
  #: -> String
  def def4
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

Const2 = Class.new do
  #: -> String
  def def5
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  #: -> String
  def def6
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var1 = 42
var1 &&= Class.new do
  #: -> String
  def def7
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var2 = 42
var2 &&= Class.new do
  #: -> String
  def def8
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  #: -> String
  def def9
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var3 ||= Class.new do
  #: -> String
  def def10
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var4 ||= Class.new do
  #: -> String
  def def11
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  #: -> String
  def def12
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var3 += Class.new do # error: Method `+` does not exist on `T::Class[T.untyped]`
  #: -> String
  def def13
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var4 += Class.new do # error: Method `+` does not exist on `T::Class[T.untyped]`
  #: -> String
  def def14
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  #: -> String
  def def15
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

begin
  #: -> String
  def def16
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  #: -> String
  def def17
    nil # error: Expected `String` but found `NilClass` for method result type
  end

  #: -> String
  def def18
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

#: -> Symbol
def def19
  #: -> String
  def def20
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

#: -> Symbol
def self.def21
  #: -> String
  def def22
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

for i in 1..10
  #: -> String
  def def23
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

[
  #: -> String
  def def24
    nil # error: Expected `String` but found `NilClass` for method result type
  end
]

begin
  #: -> String
  def def25
    nil # error: Expected `String` but found `NilClass` for method result type
  end
rescue
  #: -> String
  def def26
    nil # error: Expected `String` but found `NilClass` for method result type
  end
ensure
  #: -> String
  def def27
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

if true
  #: -> String
  def def28
    nil # error: Expected `String` but found `NilClass` for method result type
  end
else
  #: -> String
  def def29
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

var5, var6 = begin
  #: -> String
  def def30
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

case ARGV.first
when "foo"
  #: -> String
  def def31
    nil # error: Expected `String` but found `NilClass` for method result type
  end
else
  #: -> String
  def def32
    nil # error: Expected `String` but found `NilClass` for method result type
  end
end

class Foo
  class << self
    #: -> String
    def def33
      nil # error: Expected `String` but found `NilClass` for method result type
    end
  end
end
