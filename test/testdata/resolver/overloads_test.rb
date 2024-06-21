# typed: true
extend T::Sig

sig {params(s: Integer).void}
sig {params(s: String).void}
def f(s); end # error: against an overloaded signature

sig {params(s: Symbol).void}
sig {params(s: Float).void}
def f(s); end # error: against an overloaded signature

f(0) # error: Expected `Symbol`
f('') # error: Expected `Symbol`
f(:s)
f(0.0)

class Wrap1
  extend T::Sig

  sig {params(x: Integer).void}
  sig {params(x: Integer, blk: T.proc.returns(Integer)).void}
  def one_kw(x:, &blk); end # error: against an overloaded signature

  sig {params(x: Integer).void}
  sig {params(x: Integer, blk: T.proc.returns(Integer)).void}
  def opt_kw(x: 0, &blk); end # error: against an overloaded signature

  sig {params(s: String, x: Integer).void}
  sig {params(s: String, x: Integer, blk: T.proc.returns(Integer)).void}
  def opt_pos_opt_kw(s='', x: 0, &blk); end # error: against an overloaded signature

  sig {params(x: Integer, y: String).void}
  sig {params(x: Integer, y: String, blk: T.proc.returns(Integer)).void}
  def two_kw(x:, y:, &blk); end # error: against an overloaded signature

  sig {params(x: Integer, y: String).void}
#             ^ error: Overloaded functions cannot have keyword arguments
#                         ^ error: Unknown argument name
  sig {params(x: Integer, y: String, blk: T.proc.returns(Integer)).void}
#             ^ error: Overloaded functions cannot have keyword arguments
#                         ^ error: Unknown argument name
  def arg_in_sig_but_not_method(x:, &blk); end # error: against an overloaded signature

  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  sig {params(y: String, x: Integer).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  def keyword_ordering_matters(x:, y:); end # error: against an overloaded signature
  #                            ^^ error: Bad parameter ordering
  #                                ^^ error: Bad parameter ordering

  sig {params(x: T::Boolean).void}
#             ^ error: Overloaded functions cannot have keyword arguments
  sig {params(x: FalseClass, blk: T.proc.returns(Integer)).void}
#             ^ error: Overloaded functions cannot have keyword arguments
  def arg_types_must_match(x:, &blk); end # error: against an overloaded signature

  sig {params(x: Integer, blk: T.proc.returns(Integer)).void}
#             ^ error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, blk: T.proc.returns(String)).void}
#             ^ error: Overloaded functions cannot have keyword arguments
  def only_one_block_per_pair(x:, &blk); end # error: against an overloaded signature

  sig {params(x: Integer).void}
#             ^ error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer).void}
#             ^ error: Overloaded functions cannot have keyword arguments
  def needs_one_block_per_pair(x:); end # error: against an overloaded signature

  # We currently require type equivalence for parameters in sigs for overloaded methods
  # with keyword arguments.  This is probably a little too strong, but fixing it would
  # require some careful thought.
  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U), y: Integer) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U), y: Integer, blk: T.proc.returns(Integer)) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  def matching_positional_type_parameters_not_allowed(x, y:, &blk); end # error: against an overloaded signature

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U), y: Integer) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    type_parameters(:V)
      .params(x: T.type_parameter(:V), y: Integer, blk: T.proc.returns(Integer)) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  def mismatched_positional_type_parameters_not_allowed(x, y:, &blk); end # error: against an overloaded signature

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U)) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U), blk: T.proc.returns(Integer)) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  def matching_kwarg_type_parameters_not_allowed(x:, &blk); end # error: against an overloaded signature

  sig do
    type_parameters(:U)
      .params(x: T.type_parameter(:U)) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    type_parameters(:V)
      .params(x: T.type_parameter(:V), blk: T.proc.returns(Integer)) # error: Overloaded functions cannot have keyword arguments
      .void
  end
  def mismatched_kwargs_type_parameters_not_allowed(x:, &blk); end # error: against an overloaded signature
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

  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  def missing_kw1(x:, y: ''); end # error: against an overloaded signature

  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  def missing_kw2(x:, y: ''); end # error: against an overloaded signature

  # Same pattern as the above, but 3 sigs will error where two will not.
  # TODO(froydnj): can we test what happens when the relevant error is suppressed?
  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, y: String).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer, y: String, z: Float).void} # error-with-dupes: Overloaded functions cannot have keyword arguments
  def multiple_missing_kw(x:, y: '', z: 0.0); end # error: against an overloaded signature

  sig {params(z: String, x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  def mismatched_positional_args1(z='', x:); end # error: against an overloaded signature

  sig {params(x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  sig {params(z: String, x: Integer).void} # error: Overloaded functions cannot have keyword arguments
  def mismatched_positional_args2(z='', x:); end # error: against an overloaded signature
end

Wrap2.new.missing_kw1(x: 0, y: 'foo') # error: Unrecognized keyword argument
Wrap2.new.missing_kw1(x: 0)
Wrap2.new.missing_kw2(x: 0, y: 'foo') # error: Unrecognized keyword argument
Wrap2.new.missing_kw2(x: 0)

Wrap2.new.multiple_missing_kw(x: 1)
Wrap2.new.multiple_missing_kw(x: 1, y: 'bar', z: 0.1) # error-with-dupes: Unrecognized keyword argument

# These errors are wrong; they are artifacts of overload matching ignoring keyword arguments.
Wrap2.new.mismatched_positional_args1('z', x: 0) # error: Too many positional arguments
Wrap2.new.mismatched_positional_args1(x: 0)
Wrap2.new.mismatched_positional_args2('z', x: 0) # error: Too many positional arguments
Wrap2.new.mismatched_positional_args2(x: 0)

class WrapGeneric1
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig do
    params(x: Elem, y: Integer)
      .void
  end
  sig do
    params(x: Elem, y: Integer, blk: T.proc.returns(Integer))
      .void
  end
  def matching_positional_type_members(x, y:, &blk); end # error: against an overloaded signature

  sig do
    params(x: Elem)
      .void
  end
  sig do
    params(x: Elem, blk: T.proc.returns(Integer))
      .void
  end
  def matching_kwarg_type_members(x:, &blk); end # error: against an overloaded signature
end

class WrapGeneric2
  extend T::Sig
  extend T::Generic

  Elem = type_template

  sig do
    params(x: Elem, y: Integer)
      .void
  end
  sig do
    params(x: Elem, y: Integer, blk: T.proc.returns(Integer))
      .void
  end
  def self.matching_positional_type_templates(x, y:, &blk); end # error: against an overloaded signature

  sig do
    params(x: Elem)
      .void
  end
  sig do
    params(x: Elem, blk: T.proc.returns(Integer))
      .void
  end
  def self.matching_kwarg_type_templates(x:, &blk); end # error: against an overloaded signature
end

class WrapGeneric3
  extend T::Sig
  extend T::Generic

  Elem = type_member
  Mele = type_member

  sig do
    params(x: Elem, y: Integer)
#                   ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    params(x: Mele, y: Integer, blk: T.proc.returns(Integer))
#                   ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  def mismatched_positional_type_members(x, y:, &blk); end # error: against an overloaded signature

  sig do
    params(x: Elem)
#          ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    params(x: Mele, blk: T.proc.returns(Integer))
#          ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  def mismatched_kwarg_type_members(x:, &blk); end # error: against an overloaded signature
end

class WrapGeneric4
  extend T::Sig
  extend T::Generic

  Elem = type_template
  Mele = type_template

  sig do
    params(x: Elem, y: Integer)
#                   ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    params(x: Mele, y: Integer, blk: T.proc.returns(Integer))
#                   ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  def self.mismatched_positional_type_templates(x, y:, &blk); end # error: against an overloaded signature

  sig do
    params(x: Elem)
#          ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  sig do
    params(x: Mele, blk: T.proc.returns(Integer))
#          ^ error: Overloaded functions cannot have keyword arguments
      .void
  end
  def self.mismatched_kwarg_type_templates(x:, &blk); end # error: against an overloaded signature
end


