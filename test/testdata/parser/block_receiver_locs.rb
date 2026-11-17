# typed: true
extend T::Sig

# Every expression below starts with a method call that has a block. The
# expression's loc should start where that method call starts, not at its block.
# (The parser comparison checks every loc against the Prism parser's.)

def statements(a)
  [1].to_h { |g| [g, g] }.reject { |k, _| k.nil? }
  a.map { 1 } && 2
  a.find { true }&.dig(0)
  a.map { 1 }[0]
  a.map { 1 } + [2]
  a.map { 1 }.x = 3
  a.map { 1 }.size += 1
  a.map { 1 } ? 1 : 2
  a.map { 1 }..2
  a.map { 1 } rescue nil
  a.map { 1 } if a
  a.each_slice(2) do |x| x end.to_a
  b = a.map { 1 }.first
  -> { 1 }.call
  puts a.map { 1 }.first
end

class Parent
  def child; end
end

class Child < Parent
  def child
    super { 1 }.x
  end
end

sig { returns(T::Boolean) }
def receiver_autocorrect
  [1].map { |x| x + 1 }.first.even?
  #                           ^^^^^ error: Method `even?` does not exist on `NilClass` component of `T.nilable(Integer)`
end

sig { returns(String) }
def result_type
  [1].map { |x| x + 1 }.first
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Expected `String` but found `T.nilable(Integer)` for method result type
end
