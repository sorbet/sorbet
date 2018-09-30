# typed: true
class A
end
module B
end

extend T::Helpers

sig {params(log: T.all(A, B)).void}
def consume(log)
end

sig {params(log: T.nilable(T.all(A, B))).void}
def meth(log)
  if log
    consume(T.must(log))
  end
end
