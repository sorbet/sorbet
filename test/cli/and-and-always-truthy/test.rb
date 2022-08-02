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
end

def example2
  if always_false && T.unsafe(true)
  end
end
