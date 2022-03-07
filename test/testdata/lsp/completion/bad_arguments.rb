# typed: true

def foo(*args, **kwargs); end

def various_bad_commas_in_send(a, b, x, y)
  foo(, x: x) # error: unexpected token ","
  #   ^ completion: a, b, x, y, ...

  # This one is a little clowny unfortunately: completes even immediately
  # before and after the `a` with non-`a` results because the parser has thrown
  # away the `a` and replaced it with an ::<ErrorNode>. Which is annoying,
  # because it'd really be convenient to take advantage of this one to
  # potentially suggest available keyword args.
  foo(x: x, a)
  #        ^ completion: a, b, x, y, ...
  #         ^ completion: alias, and, a, ...
  #          ^ completion: alias, and, a, ...
  #         ^ error: Method `a` does not exist
  #         ^ error: positional arg "a" after keyword arg

  foo(a, , y: y) # error: unexpected token ","
  #      ^ completion: a, b, x, y, ...

  # This one was kind of an accident of the implementation for the next one. I
  # don't think it's too bad if the behavior here needs to change in the future.
  foo(a, x: y:)
  #      ^^ error: unexpected token tLABEL

  # Either another positional arg, or the start of a keyword
  foo(a, x y: y)
  #       ^ completion: x, ...
  #      ^ error: Method `x` does not exist
  #       ^ error: missing token ","

  # Inserting new keyword arg into list
  foo(a, x: y: y) # error: unexpected token tLABEL
  #        ^ completion: a, b, x, y, ...
  #         ^ completion: a, b, x, y, ...

  foo(a, x: x, , y: y) # error: unexpected token ","
  #            ^ completion: a, b, x, y, ...

  foo(a, x: , y: y)
  #         ^ completion: a, b, x, y, ...


  foo(x: x y: y)
  #       ^ completion: x, ...
  #      ^ error: Method `x` does not exist
  #       ^ error: missing token ","
  foo(a: a, x: x y: y)
  #             ^ completion: x, ...
  #            ^ error: Method `x` does not exist
  #             ^ error: missing token ","
end
