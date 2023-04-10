# typed: true
# experimental-ruby3-keyword-args: true

extend T::Sig

sig {params(x: Integer, y: Integer).void}
def takes_kwargs(x, y:)
end

arghash = {y: 42}
takes_kwargs(99, arghash) # error: Keyword argument hash without `**` is deprecated

takes_kwargs(99, **arghash)

sig {params(h: T::Hash[T.untyped, T.untyped], kw: T::Hash[T.untyped, T.untyped]).void}
def takes_only_hash_and_kwargs(h, **kw); end

takes_only_hash_and_kwargs(a: "x")
                        #  ^^^^^^ error: Passing the keyword argument as the last hash parameter is deprecated
takes_only_hash_and_kwargs({a: "x"}) # => OK

sig {params(h: T::Hash[T.untyped, T.untyped]).void}
def takes_only_hash(h); end

takes_only_hash(a: "x") # => OK
takes_only_hash({a: "x"}) # => OK

sig {params(k: Integer).void}
def takes_only_kwargs(k:) end

takes_only_kwargs(k: 42) # => OK
takes_only_kwargs({k: 42})
                # ^^^^^^^ error: Keyword argument hash without `**` is deprecated


module A
  extend T::Sig
  sig {params(a: String, b: String, h: T::Hash[String, Integer], kw: T::Hash[String, Integer]).void}
  def self.takes_hash(a, b, h, **kw); end
end

A.takes_hash("", "", h: 42)
                   # ^^^^^ error: Passing the keyword argument as the last hash parameter is deprecated
A.takes_hash("", "", {h: 42}) # => OK


sig {params(a: Integer, b: String).void}
def needs_double_splats(a, b:); end

options = { b: "str" }

needs_double_splats(42, options)
                      # ^^^^^^^ error: Keyword argument hash without `**` is deprecated
needs_double_splats(42, **options) # => OK



sig {params(arg: T::Hash[T.untyped, T.untyped], arg_with_default: String ).void}
def last_arg_with_default(arg, arg_with_default: "something"); end

last_arg_with_default(item: "42")
                    # ^^^^^^^^^^ error: Passing the keyword argument as the last hash parameter is deprecated
last_arg_with_default({ item: "42" }) # => OK


sig {params(arg0: Integer, arg: T::Hash[T.untyped, T.untyped], arg_with_default: String ).void}
def last_arg_with_default2(arg0, arg, arg_with_default: "something"); end

last_arg_with_default2(1, item: 42)
                        # ^^^^^^^^ error: Passing the keyword argument as the last hash parameter is deprecated
sig {params(kwargs: T::Hash[T.untyped, T.untyped]).void}
def kwargs_with_default(kwargs = {}); end

kwargs_with_default(k: 1) # => OK
kwargs_with_default({k: 1}) # => OK

sig {params(a: Integer, h: T::Hash[T.untyped, T.untyped]).void}
def takes_empty_hash(a, h); end

empty_hash = {}
takes_empty_hash(1, **empty_hash)
                  # ^^^^^^^^^^^^ error: Passing the keyword argument as the last hash parameter is deprecated

takes_empty_hash(1, empty_hash) # => OK

[{a: 42, b: "something"}].each do |a:, b:|
                                #  ^^^^^^ We don't report this usage as an error yet
  p [a, b]
end

# Examples below are similar to the real world cases
# which gave me false-positive or false-negative result.
# Keeping them to catch regressions

sig {params(url: String, key: T.nilable(String), params: T::Hash[T.untyped, T.untyped], env: T::Hash[T.untyped, T.untyped]).void}
def get_live(url, key = nil, params: {}, env: {}); end

get_live("some url") # => OK


sig do
  params(
    id: T.nilable(T.any(String, Integer)),
    extra: T::Hash[T.untyped, T.untyped],
    opts: T::Hash[T.untyped, T.untyped]
  )
    .void
end
def load!(id, extra = {}, opts = {}); end


load!("some_id") # => OK


def assert_match(matcher, obj, msg = nil); end

sig {returns(T.untyped)}
def returns_untyped; end;

assert_match(/some regex/, returns_untyped) # => OK


class LoadAll
  extend T::Sig

  sig do
    params(
      query: T::Hash[T.untyped, T.untyped],
      opts: T::Hash[T.untyped, T.untyped],
      opts2: T::Hash[T.untyped, T.untyped],
      blk: T.untyped
      ).returns(T.attached_class)
  end
  def self.load_all(query = {}, opts = {}, opts2 = {}, &blk); new end



  sig do
    params(
      query: T::Hash[T.untyped, T.untyped],
      opts: T::Hash[T.untyped, T.untyped],
      blk: T.untyped
      ).returns(T.attached_class)
  end
  def self.load_one(query = {}, opts = {}, &blk); new end
end

LoadAll.load_all({m: "m_42", payment_intent: "pi_43"}) # => OK
LoadAll.load_one({m: "m_42", payment_intent: "pi_43"}) # => OK

class GQLObject
  extend T::Sig
  sig do
    params(
      field_name: Symbol,
      type: T.untyped,
      desc: T.nilable(::String),
      null: T.nilable(T::Boolean),
      unnamed_kwargs: T.untyped,
      block: T.nilable(T.proc.void)
    )
      .void
  end
  def self.field(
    field_name,
    type,
    desc = nil,
    null: nil,
    **unnamed_kwargs,
    &block
  )
  end

end

class Obj < GQLObject
  field :transaction_id, String
end


# `prop` methods receive an additional keyword arg, because of the rewriter step
class MyProp
  extend T::Sig

  sig do
    params(
      name: Symbol,
      prop_type: T.untyped,
      rules: T::Hash[Symbol, T.untyped],
      arg0: T.untyped,
      arg1: T.nilable(String),
      arg2: T.untyped,
      arg3: T.nilable(String),
      arg4: T.nilable(String),
      arg5: T.nilable(String),
      arg6: T.untyped,
      arg7: T.nilable(T.proc.returns(T.untyped)),
      arg8: T.nilable(T::Array[String]),
      arg9: T.nilable(T.proc.returns(T.nilable(T::Array[String]))),
      arg10: T.nilable(T.proc.returns(T.untyped)),
      arg11: T::Boolean,
      kwargs: T.untyped
    )
      .returns(T.untyped)
  end
  def self.prop(
    name,
    prop_type,
    rules = {},
    arg0: nil,
    arg1: nil,
    arg2: nil,
    arg3: nil,
    arg4: nil,
    arg5: nil,
    arg6: nil,
    arg7: nil,
    arg8: nil,
    arg9: nil,
    arg10: nil,
    arg11: false,
    **kwargs
  )
  end
end

class Abacaba < MyProp
    prop :_id, String, arg2: "e.g., _id"
    prop :a, T::Array[String]
    prop :b, T::Array[String], arg2: "bbbb"
    prop :d, Integer, arg1: "Amount in cents", arg2: "e.g. 1000"
end

sig do
  params(
    name: String,
    tags: T::Hash[T.untyped, T.untyped],
    x: Integer,
    blk: T.proc.returns(String)
  )
  .returns(String)
end
def foo(name, tags: {}, x: 42, &blk)
  yield
end

blk = Proc.new
foo("", tags: {}, x: 1, &blk)
