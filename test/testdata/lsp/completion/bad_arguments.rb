# typed: true

# TODO(jez) Also the parser output before+after

def foo(*args, **kwargs); end

def various_bad_commas_in_send(a, b, x, y)
  foo(, x: x) # error: unexpected token ","
  #   ^ completion: a, b, x, y, ...

  # This one is a little clowny unfortunately: completes even immediately
  # before and after the `a` with non-`a` results because the parser has thrown
  # away the `a` and replaced it with an ::<ErrorNode>. Which is annoying,
  # because it'd really be convenient to take advantage of this one to
  # potentially suggest available keyword args.
  # TODO(jez) Actually, this might not be that hard to special case?
  foo(x: x, a) # error: unexpected token ")"
  #        ^ completion: a, b, x, y, ...
  #         ^ completion: a, b, x, y, ...
  #          ^ completion: a, b, x, y, ...


  foo(a, , y: y) # error: unexpected token ","
  #      ^ completion: a, b, x, y, ...

  # This one was kind of an accident of the implementation for the next one. I
  # don't think it's too bad if the behavior here needs to change in the future.
  foo(a, x: y:)

  # Inserting new keyword arg into list
  foo(a, x: y: y)
  #        ^ completion: todo
  #         ^ completion: todo

  foo(a, x: x, , y: y)
  #            ^ completion: todo

  foo(a, x: , y: y)
  #         ^ completion: todo

  # TODO(jez) these are kind of tricky, which is unfortunate becuase they're
  # also important for completion
  # foo(a, x y: y)
  # foo(a, x: x y: y)
end
