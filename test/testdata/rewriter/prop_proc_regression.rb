# typed: true

class Foo < T::Struct
    prop :proc, T.proc.params(x: Integer).void
    prop :proc_type, type: T.proc.params(x: Integer).void
end

T.reveal_type(Foo.new(proc: lambda {|x| nil }, proc_type: lambda {|x| nil})) # error: Revealed type: `Foo`
