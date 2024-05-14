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
  const :baz, String
# ^^^^^ apply-code-action: [A] Extract Variable
end

def endless_method = 1 + 123
#                        ^^^ apply-code-action: [A] Extract Variable

# The actual issue we saw causing crashes was the following, the selection is the entire method body:
# def foo
#   1 + 1
#   2 + 2
# end
# However, our tests don't support multi-line assertions, so this test case is approximating that
# by joining the lines with a ; (the parse result will be the same for both cases).
# TODO(neil): add a multiline test case here once we have multiline assertions.

def multi_stat_selection
  1 + 1; 2 + 2
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable
end
