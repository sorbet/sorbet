# typed: true

# TODO(jez) Fix this test

class A
  def foo(aaa, bbb)
    # trailing space is intentional
    if
    #  ^ completion: (nothing)
    puts 'after'
  end # error: Hint: closing "end" token was not indented as far as "if" token
end # error: unexpected token "end of file"
