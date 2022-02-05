# typed: false

# We still don't recover from this error as much as we'd like to.
#
# Some options for handling this better:
# - tweak the parser somehow?
# - make sure that cancellable slow path is really good
# - drive people towards using completion snippets for `if` / `else` / `end`

class A
  def foo
    if true
  end
end

class B
  def bar
  end
end # error: unexpected token
