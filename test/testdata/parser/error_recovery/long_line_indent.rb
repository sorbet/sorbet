# typed: false
def foo
  really_long_method_________________________________________________________________________________________________________ do
  #                                                                                                                           ^^ error: Hint: this "do" token might not be properly closed
end # error: unexpected token "end of file"
