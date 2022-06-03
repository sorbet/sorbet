# typed: true

A=3 # error: Expected `Runtime object representing type: T.untyped` but found `Integer(3)` for field
A=type_member # error: `type_member` cannot be used at the top-level
