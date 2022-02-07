# typed: true

def various_bad_commas_in_array(a, b, x, y)
  [, x] # error: unexpected token ","
  #^ completion: a, b, x, y, ...

  [x y] # error: unexpected token tIDENTIFIER
  # TODO(jez) completion?

  [x, , y] # error: unexpected token ","
  #   ^ completion: a, b, x, y, ...

  [x, y, ,] # error: unexpected token ","
  #      ^ completion: a, b, x, y, ...
end
