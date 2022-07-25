# typed: true
extend T::Sig

sig {params(s: Integer).void}
sig {params(s: String).void}
def f(s); end

sig {params(s: Symbol).void}
sig {params(s: Float).void}
def f(s); end

f(0) # error: Expected `Symbol`
f('') # error: Expected `Symbol`
f(:s)
f(0.0)

class Wrap1
  extend T::Sig

  sig {params(x: Integer).void}
  sig {params(x: Integer, blk: T.proc.returns(Integer)).void}
  def one_kw(x:, &blk); end

  sig {params(x: Integer).void}
  sig {params(x: Integer, blk: T.proc.returns(Integer)).void}
  def opt_kw(x: 0, &blk); end

  sig {params(s: String, x: Integer).void}
  sig {params(s: String, x: Integer, blk: T.proc.returns(Integer)).void}
  def opt_pos_opt_kw(s='', x: 0, &blk); end

  sig {params(x: Integer, y: String).void}
  sig {params(x: Integer, y: String, blk: T.proc.returns(Integer)).void}
  def two_kw(x:, y:, &blk); end

  sig {params(x: Integer, y: String).void} # error: Unknown argument name
  sig {params(x: Integer, y: String, blk: T.proc.returns(Integer)).void} # error: Unknown argument name
  def arg_in_sig_but_not_method(x:, &blk); end

  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  sig {params(y: String, x: Integer).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  def keyword_ordering_matters(x:, y:); end # error-with-dupes: Bad parameter ordering
end

Wrap1.new.opt_kw()
Wrap1.new.opt_kw(x: 1)
Wrap1.new.opt_pos_opt_kw()
Wrap1.new.opt_pos_opt_kw('foo')
Wrap1.new.opt_pos_opt_kw('foo', x: 5)
Wrap1.new.opt_pos_opt_kw(x: 6)
Wrap1.new.opt_pos_opt_kw(x: 7.0) # error: Expected `Integer` but found

class Wrap2
  extend T::Sig

  # Furthermore, the errors at infer time are somewhat nonsensical.
  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  def missing_kw1(x:, y: ''); end

  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  def missing_kw2(x:, y: ''); end

  # Same pattern as the above, but 3 sigs will error where two will not.
  # TODO(froydnj): can we test what happens when the relevant error is suppressed?
  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, y: String, z: Float).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  def multiple_missing_kw(x:, y: '', z: 0.0); end
end

Wrap2.new.missing_kw1(x: 0, y: 'foo') # error: Unrecognized keyword argument
Wrap2.new.missing_kw1(x: 0)
Wrap2.new.missing_kw2(x: 0, y: 'foo') # error: Unrecognized keyword argument
Wrap2.new.missing_kw2(x: 0)

Wrap2.new.multiple_missing_kw(x: 1)
Wrap2.new.multiple_missing_kw(x: 1, y: 'bar', z: 0.1) # error-with-dupes: Unrecognized keyword argument
