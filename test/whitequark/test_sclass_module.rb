# typed: true

module << foo; nil; end
#      ^^ parser-error: unexpected token "<<"
#                   ^^^ parser-error: unexpected token "end"
