# typed: true
extend T::Sig

def example1(&b)
  prc = -> {}
  does_not_exist!(&prc) # error: Method `does_not_exist!` does not exist on `Object`

  does_not_exist!(&b) # error: Method `does_not_exist!` does not exist on `Object`

  does_not_exist!(*[1], &b) # error: Method `does_not_exist!` does not exist on `Object`
end

sig {params(b: NilClass).void}
def example2(&b)
  does_not_exist!(&b) # error: Method `does_not_exist!` does not exist on `Object`

  does_not_exist!(*[1], &b) # error: Method `does_not_exist!` does not exist on `Object`
end

sig {params(b: T.proc.void).void}
def example3(&b)
  does_not_exist!(&b) # error: Method `does_not_exist!` does not exist on `Object`

  does_not_exist!(*[1], &b) # error: Method `does_not_exist!` does not exist on `Object`
end

sig {params(b: T.nilable(T.proc.void)).void}
def example4(&b)
  does_not_exist!(&b) # error: Method `does_not_exist!` does not exist on `Object`

  does_not_exist!(*[1], &b) # error: Method `does_not_exist!` does not exist on `Object`
end

