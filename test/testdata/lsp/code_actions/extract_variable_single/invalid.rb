# typed: true
# selective-apply-code-action: refactor.extract
# enable-experimental-lsp-extract-to-variable: true
# assert-no-code-action: refactor.extract
def example(x)
#           ^ apply-code-action: [A] Extract Variable (this occurrence only)
  xs = []
# ^^ apply-code-action: [A] Extract Variable (this occurrence only)
  xs = []
#      ^ apply-code-action: [A] Extract Variable (this occurrence only)
  xs.each { |x| return x.foo if x.foo}
#                                ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  T.must(123)
#   ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  T.must(123)
# ^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  return
# ^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
end

def extract_method_parameters(x, y); end
#                             ^ apply-code-action: [A] Extract Variable (this occurrence only)

def extract_method_parameters_2(x, y); end
#                                  ^ apply-code-action: [A] Extract Variable (this occurrence only)

# Extract block parameters

[].each do |x, y| end
#           ^ apply-code-action: [A] Extract Variable (this occurrence only)

[].each do |x, y| end
#              ^ apply-code-action: [A] Extract Variable (this occurrence only)

class A
  attr_accessor :a

  x = 1 + 1
#     ^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  x = 1 + 1
#       ^ apply-code-action: [A] Extract Variable (this occurrence only)
  x = 1 + 1
# ^ apply-code-action: [A] Extract Variable (this occurrence only)

  def multiple_return_break_next
    1.times do
      if 1 == 2
        break 1, 2
#       ^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
      else
        next 1, 2
#       ^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
      end
    end
    return 1, 2
#   ^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  end
end

  A.new.a &&= 1
# ^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

  A.new.a ||= 1
# ^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

  A.new.a *= 1
# ^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

a = T.let(1, T.untyped)

  a &&= 1
# ^ apply-code-action: [A] Extract Variable (this occurrence only)

  a ||= 1
# ^ apply-code-action: [A] Extract Variable (this occurrence only)

  a *= 1
# ^ apply-code-action: [A] Extract Variable (this occurrence only)

a = T.unsafe(nil) || T.unsafe(nil)
#                ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

a = T.unsafe(nil) && T.unsafe(nil)
#                ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

  A.new.a&.foo &&= 1
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

  A.new.a&.foo ||= 1
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

  A.new.a&.foo *= 1
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)

[1, 2, 3].each do |x|
#         ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
end
[1, 2, 3].each do |x|
#                  ^ apply-code-action: [A] Extract Variable (this occurrence only)
  break
# ^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
  next
# ^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
end

begin
rescue Errno::ENOENT
#      ^^^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
rescue ArgumentError => e
#      ^^^^^^^^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
rescue Exception => e
#                   ^ apply-code-action: [A] Extract Variable (this occurrence only)
end

class B < T::Struct
  const :baz, String
# ^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
end

def endless_method = 1 + 123
#                        ^^^ apply-code-action: [A] Extract Variable (this occurrence only)

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
# ^^^^^^^^^^^^ apply-code-action: [A] Extract Variable (this occurrence only)
end
