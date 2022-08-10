# typed: strict
# spacer for exclude-from-file-update

int_to_str = T.let(->(x) {x.to_s}, T.proc.params(arg0: Integer).returns(String))
str_to_int = T.let(->(x) {x.to_s}, T.proc.params(arg0: String).returns(Integer))

A.takes_blk(&int_to_str)
A.takes_blk(&str_to_int) # error: Expected `T.proc.params(arg0: Integer).returns(String)` but found `T.proc.params(arg0: String).returns(Integer)` for block argument
