# typed: false
# disable-fast-path: true
  T = T.type_alias
# ^^^^^^^^^^^^^^^^ error: Redefining constant `T` as a static field
    # ^^^^^^^^^^^^ error: No block given to `T.type_alias`
