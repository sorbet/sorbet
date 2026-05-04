# typed: false
  def
# ^^^ error: Hint: this "def" token might not be followed by a method name
# ^^^ error: Hint: this "def" token is not closed before the end of the file
  ensure # error: "end of file"
