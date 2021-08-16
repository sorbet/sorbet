# typed: true

[1] in [a]; T.reveal_type(a)
          # ^^^^^^^^^^^^^^^^ error: Revealed type: `T.untyped`
[1] => [a, b]; T.reveal_type(a); T.reveal_type(b)
             # ^^^^^^^^^^^^^^^^ error: Revealed type: `T.untyped`
             #                   ^^^^^^^^^^^^^^^^ error: Revealed type: `T.untyped`
