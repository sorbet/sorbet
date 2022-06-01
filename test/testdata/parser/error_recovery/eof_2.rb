# typed: true

def foo # error: Hint: this "def" token is not closed before the end of the file
  def bar # error: Hint: this "def" token is not closed before the end
    def qux # error: Hint: this "def" token is not closed before the end
      puts('inside qux')

    puts('inside bar')

  puts('inside foo') # error: unexpected token "end of file"
