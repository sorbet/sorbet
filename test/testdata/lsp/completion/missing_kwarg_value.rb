# typed: true
def takes_keyword_args(x:)
end


def calls_with_keyword_args(x, y)
  # TODO(jez) The error below is wrong, and should be fixed when we get proper
  # Ruby 3.1 keyword punning support.
  takes_keyword_args(x: )
  #                  ^ error: Method `x` does not exist
  #                     ^ completion: x, y, ...
  #                    ^ completion: x, y, ...
end
