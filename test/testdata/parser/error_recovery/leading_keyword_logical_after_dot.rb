# typed: false

def keyword_and(x, y)
  x.
  and y # error: unexpected token
  after_and
end

def keyword_or(x, y)
  x.
  or y # error: unexpected token
  after_or
end

# These exercise recovery when a comment appears between the trailing dot and
# the keyword on the next line.
def commented_keyword_and(x, y)
  x. # trailing comment
  and y # error: unexpected token
  after_commented_and
end

def commented_keyword_or(x, y)
  x. # trailing comment
  or y # error: unexpected token
  after_commented_or
end
