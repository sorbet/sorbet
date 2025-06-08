# typed: true

# TODO(jez) Fix this test

class A
  def foo(aaa, bbb)
    # trailing space is intentional
    if # parser-error: Hint: this "if" token might not be properly closed
    #  ^ completion: (nothing)
    puts 'after'
  end
end # parser-error: unexpected token "end of file"
