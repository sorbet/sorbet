# typed: true
extend T::Sig

sig {returns(TrueClass)}
def always_true; true; end

sig {returns(FalseClass)}
def always_false; false; end

def example1
  if always_true && T.unsafe(true)
    puts 'here'
  end

  if always_true || T.unsafe(true)
    puts 'here'
  end
end

def example2
  if always_false && T.unsafe(true)
  end

  if always_false || T.unsafe(true)
  end
end

sig {params(always_true: TrueClass).void}
def example3(always_true)
  # TODO: We don't get the same sort of nice error message in this case.
  if always_true && T.unsafe(true)
    puts 'here'
  end
end
