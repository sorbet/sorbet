# typed: false
def foo
  really_long_method_________________________________________________________________________________________________________ do
  #                                                                                                                           ^^ parser-error: Hint: this "do" token might not be properly closed
end # parser-error: unexpected token "end of file"
