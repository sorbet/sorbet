# typed: true
module Foo; end
module Bar; end
module Baz; end

with_any = T.let(T.unsafe(nil), T.any(Foo, Bar))
T.reveal_type(with_any) # error: Revealed type: `T.any(Foo, Bar)`

with_any_more = T.let(T.unsafe(nil), T.any(Foo, Bar, Baz))
T.reveal_type(with_any_more) # error: Revealed type: `T.any(Foo, Bar, Baz)`

with_any_with_nilable = T.let(T.unsafe(nil), T.any(Foo, Bar, T.nilable(Baz)))
T.reveal_type(with_any_with_nilable) # error: Revealed type: `T.nilable(T.any(Baz, Foo, Bar))`

with_all = T.let(T.unsafe(nil), T.all(Foo, Bar))
T.reveal_type(with_all) # error: Revealed type: `T.all(Foo, Bar)`

with_all_more = T.let(T.unsafe(nil), T.all(Foo, Bar, Baz))
T.reveal_type(with_all_more) # error: Revealed type: `T.all(Foo, Bar, Baz)`

with_all_nested = T.let(T.unsafe(nil), T.all(Foo, T.all(Bar, Baz)))
T.reveal_type(with_all_nested) # error: Revealed type: `T.all(Bar, Baz, Foo)`
