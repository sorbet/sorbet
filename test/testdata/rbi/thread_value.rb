# typed: true

# `Thread#value` returns whatever the block returned, which is not necessarily
# an `Object` (a block may return a `BasicObject`), so it is typed `T.untyped`
# rather than `Object`.
int_thread = Thread.new { 2 + 2 }
T.reveal_type(int_thread.value) # error: Revealed type: `T.untyped`

basic_thread = Thread.new { BasicObject.new }
T.reveal_type(basic_thread.value) # error: Revealed type: `T.untyped`

# `T.untyped` keeps the common case ergonomic (no cast needed):
res = int_thread.value + 1
T.reveal_type(res) # error: Revealed type: `T.untyped`
