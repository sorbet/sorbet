# typed: false

if /wat/
#  ^^^^^ error: Unsupported node type `MatchCurLine`
  "This is _not_ a truthiness test of a Regexp literal,"
  "but a special syntax implicitly match against the last line read by an IO object."
end

if // # testing an empty pattern
#  ^^ error: Unsupported node type `MatchCurLine`
end
