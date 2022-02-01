# typed: true
extend T::Sig

sig {type_parameters(:)}
def ex1; end

sig {type_parameters(x)}
def ex2; end

sig {type_parameters(X)}
def ex3; end

y = nil
sig {type_parameters(y)}
def ex4; end
