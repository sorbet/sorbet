# typed: true

for i in 1..7 # error: Hint: this "for" token is not closed before the end of the file
for j in 1..2
end # error: unexpected token "end of file"
