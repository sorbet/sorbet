# typed: true
class Object < BasicObject
  RUBY_VERSION = T.let(T.unsafe(nil), String)
end

class A
  def bar
    RUBY_VERSION
  end
end

puts A.new.bar
