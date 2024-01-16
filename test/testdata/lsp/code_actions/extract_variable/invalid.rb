# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true
# assert-no-code-action: refactor.extract
def example(x)
#           ^ apply-code-action: [A] Extract Variable
  xs = []
# ^^ apply-code-action: [A] Extract Variable
  xs = []
#      ^ apply-code-action: [A] Extract Variable
  xs.each { |x| return x.foo if x.foo}
#                                ^^^^ apply-code-action: [A] Extract Variable
  T.must(123)
#   ^^^^ apply-code-action: [A] Extract Variable
  T.must(123)
# ^^^^^^ apply-code-action: [A] Extract Variable
  return
# ^^^^^^ apply-code-action: [A] Extract Variable
end

def extract_method_parameters(x, y); end
#                             ^ apply-code-action: [A] Extract Variable

def extract_method_parameters_2(x, y); end
#                                  ^ apply-code-action: [A] Extract Variable

# Extract block parameters

[].each do |x, y| end
#           ^ apply-code-action: [A] Extract Variable

[].each do |x, y| end
#              ^ apply-code-action: [A] Extract Variable

class A
  attr_accessor :a

  x = 1 + 1
#     ^^^ apply-code-action: [A] Extract Variable
  x = 1 + 1
#       ^ apply-code-action: [A] Extract Variable
  x = 1 + 1
# ^ apply-code-action: [A] Extract Variable

  def multiple_return_break_next
    1.times do
      if 1 == 2
        break 1, 2
#       ^^^^^^^^^^ apply-code-action: [A] Extract Variable
      else
        next 1, 2
#       ^^^^^^^^^^ apply-code-action: [A] Extract Variable
      end
    end
    return 1, 2
#   ^^^^^^^^^^^ apply-code-action: [A] Extract Variable
  end
end

  A.new.a &&= 1
# ^^^^^^^ apply-code-action: [A] Extract Variable

  A.new.a ||= 1
# ^^^^^^^ apply-code-action: [A] Extract Variable

  A.new.a *= 1
# ^^^^^^^ apply-code-action: [A] Extract Variable

a = T.let(1, T.untyped)

  a &&= 1
# ^ apply-code-action: [A] Extract Variable

  a ||= 1
# ^ apply-code-action: [A] Extract Variable

  a *= 1
# ^ apply-code-action: [A] Extract Variable

a = T.unsafe(nil) || T.unsafe(nil)
#                ^^^^ apply-code-action: [A] Extract Variable

a = T.unsafe(nil) && T.unsafe(nil)
#                ^^^^ apply-code-action: [A] Extract Variable

  A.new.a&.foo &&= 1
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable

  A.new.a&.foo ||= 1
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable

  A.new.a&.foo *= 1
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable

[1, 2, 3].each do |x|
#         ^^^^ apply-code-action: [A] Extract Variable
end
[1, 2, 3].each do |x|
#                  ^ apply-code-action: [A] Extract Variable
  break
# ^^^^^ apply-code-action: [A] Extract Variable
  next
# ^^^^ apply-code-action: [A] Extract Variable
end

begin
rescue Errno::ENOENT
#      ^^^^^^^^^^^^^ apply-code-action: [A] Extract Variable
rescue ArgumentError => e
#      ^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Variable
rescue Exception => e
#                   ^ apply-code-action: [A] Extract Variable
end

class B < T::Struct
  const :bar, String
#             ^^^^^^ apply-code-action: [A] Extract Variable

  const :baz, String
# ^^^^^ apply-code-action: [A] Extract Variable
end

def endless_method = 1 + 123
#                        ^^^ apply-code-action: [A] Extract Variable
