# typed: false
A::B # error: Resolving constants through type aliases is not supported
A=type_member # error: `type_member` cannot be used at the top-level
