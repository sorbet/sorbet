# typed: true

[1] in [a]; T.reveal_type(a)
          # ^^^^^^^^^^^^^^^^ error: Revealed type: `T.nilable(Integer)`
[1] => [a, b]; T.reveal_type(a); T.reveal_type(b)
             # ^^^^^^^^^^^^^^^^ error: Revealed type: `Integer(1)`
             #                   ^^^^^^^^^^^^^^^^ error: Revealed type: `T.untyped`
