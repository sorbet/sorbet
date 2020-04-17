# typed: true

class Foo < T::Struct
    prop :proc, T.proc.params(x: Integer).void
end

T.reveal_type(Foo.new(proc: lambda {|x| nil })) # error: Revealed type: `Foo`
