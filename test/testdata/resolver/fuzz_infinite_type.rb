# typed: false
# disable-fast-path: true
  T = T.type_alias
# ^ error: Unable to resolve right hand side of type alias `T`
# ^^^^^^^^^^^^^^^^ error: Redefining constant `T`
