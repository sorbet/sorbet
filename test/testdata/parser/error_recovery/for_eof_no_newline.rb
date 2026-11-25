# typed: true

# This file intentionally has no trailing newline.

for i in 1..7 # error: Hint: this "for" token is not closed before the end of the file
unless 1 == 2 # error: unexpected token "unless"