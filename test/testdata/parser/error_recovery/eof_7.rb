# typed: true

# TODO(jez) We don't do the same rewinding for begin/end blocks that we do for
# class/method blocks, so the indentation levels in the body of the begin are
# not used for the sake of backtracking at the moment.

class A # error: Hint: this "class" token is not closed before the end of the file
  def foo(x) # error: Hint: this "def" token is not closed before the end of the file
    puts('outside begin')
    begin # error: Hint: this "begin" token is not closed before the end of the file
      puts('inside begin')

    puts('between')

    begin # error: Hint: this "begin" token is not closed before the end of the file
      puts('second begin')
    rescue
      puts('inside rescue') # error: unexpected token "end of file"
