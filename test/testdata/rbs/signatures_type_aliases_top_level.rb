# typed: strict
# enable-experimental-rbs-comments: true

#: type top_level = Integer

#: -> top_level
def top_level_type_alias
  "42"
# ^^^^ error: Expected `Integer` but found `String("42")` for method result type
end

#: type another = Integer

#: -> trailing
def trailing_type_alias
  42
# ^^ error: Expected `String` but found `Integer(42)` for method result type
end

#: type trailing = String
