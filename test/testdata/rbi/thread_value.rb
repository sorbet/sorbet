# typed: true

# `Thread#value` returns whatever the block returned, which is not necessarily
# an `Object` (a block may return a `BasicObject`), so it is typed `T.untyped`
# rather than `Object`.
T.reveal_type(Thread.new { 2 + 2 }.value) # error: Revealed type: `T.untyped`
T.reveal_type(Thread.new { BasicObject.new }.value) # error: Revealed type: `T.untyped`
