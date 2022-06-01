# typed: false
# next line is indented 101 spaces
                                                                                                     def foo # error: Hint: this "def" token is not closed before the end of the file
                                                                                                       1.times do
                                                                                                     end # error: unexpected token "end of file"
