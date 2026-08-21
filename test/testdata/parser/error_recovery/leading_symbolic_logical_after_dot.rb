# typed: false

# A leading symbolic logical operator cannot be a method name for the dangling
# dot. Ensure that recovering from the dot does not consume the rest of the
# method body.
def symbolic_and(x, y)
  x.
  && y # error: unexpected token
  after_and
end

def symbolic_or(x, y)
  x.
  || y # error: unexpected token
  after_or
end
