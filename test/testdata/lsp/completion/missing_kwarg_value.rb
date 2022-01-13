# typed: true
def takes_keyword_args(x:)
end

def calls_with_keyword_args(x, y)
  takes_keyword_args(x: ) # error: unexpected token ")"
  #                     ^ completion: x, y, ...
end
