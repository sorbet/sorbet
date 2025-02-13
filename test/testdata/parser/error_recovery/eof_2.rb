# typed: true

def foo # parser-error: Hint: this "def" token is not closed before the end of the file
  def bar # parser-error: Hint: this "def" token is not closed before the end
    def qux # parser-error: Hint: this "def" token is not closed before the end
      puts('inside qux')

    puts('inside bar')

  puts('inside foo') # parser-error: unexpected token "end of file"
