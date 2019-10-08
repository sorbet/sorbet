# typed: false
# disable-fast-path: true
  T = T.type_alias
# ^^^^^^^^^^^^^^^^ error: Redefining constant `T`
    # ^^^^^^^^^^^^ error: No block given to `T.type_alias`
