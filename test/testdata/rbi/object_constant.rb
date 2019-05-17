# typed: true
RUBY_VERSION = T.let(T.unsafe(nil), String)

class A
  def bar
    RUBY_VERSION
  end
end

puts A.new.bar
