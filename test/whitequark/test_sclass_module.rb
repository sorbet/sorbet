# typed: true

module << foo; nil; end
#      ^^ error: unexpected token "<<"
#                   ^^^ error: unexpected token "end"
