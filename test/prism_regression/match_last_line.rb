# typed: false

if // # testing an empty pattern
  #^^ error: Unsupported node type `MatchCurLine`
end

if /wat/
#  ^^^^^ error: Unsupported node type `MatchCurLine`
  "This is _not_ a truthiness test of a Regexp literal,"
  "but a special syntax implicitly match against the last line read by an IO object."
end

if /wat #{123}/
#  ^^^^^^^^^^^^ error: Unsupported node type `MatchCurLine`
  "This one uses an interpolated regexp"
end

b = !(/wat/)
#     ^^^^^ error: Unsupported node type `MatchCurLine`

b = !(/wat #{123}/)
#     ^^^^^^^^^^^^ error: Unsupported node type `MatchCurLine`
