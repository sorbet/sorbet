# typed: true
# experimental-ruby3-keyword-args: true

extend T::Sig

sig {params(x: Integer, y: Integer).void}
def takes_kwargs(x, y:)
end

arghash = {y: 42}
takes_kwargs(99, arghash)
               # ^^^^^^^ error: Keyword argument hash without `**` is deprecated

takes_kwargs(99, **arghash)

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

def with_base_dir(*segments)
end

# Alerting, but shouldn't
Dir[with_base_dir("")].each do |file_path|
  p file_path
end


# Carried over from test/cli/autocorrect-kwsplat/test.rb
# For more details see https://github.com/sorbet/sorbet/pull/6771
sig {params(pos: Integer, x: Integer).void}
def requires_x(*pos, x:)
end

sig {params(pos: Integer, x: String).void}
def optional_x(*pos, x: '')
end

opts = T::Hash[Symbol, Integer].new
string_keys = T::Hash[String, Integer].new
nilble_opts = T::Hash[Symbol, T.nilable(Integer)].new
x = T.let(:x, Symbol)

# wrong error reported
requires_x(0, 1, 2, opts)
                  # ^^^^ error: Keyword argument hash without `**` is deprecated
requires_x(0, 1, 2, **opts)

requires_x(0, 1, 2, **opts, x: 0)

requires_x(0, 1, 2, x: 0, **opts)
                  # ^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated
requires_x(0, 1, 2, **{x => 0})

optional_x(0, 1, 2, **string_keys)
                  # ^^^^^^^^^^^^^ error: Expected `Symbol` but found `String` for keyword splat keys type

optional_x(0, 1, 2, **nilble_opts)
                  # ^^^^^^^^^^^^^ error: Expected `String` for keyword parameter `x` but found `T.nilable(Integer)` from keyword splat

def bar(x:, y: 10)
  p x
  p y
end

bar(x: 10)
bar(x: 10, y: 20)

args = {x: 10}
bar(**args)
bar(x: 20, **args)

other3 = T.let(:bar, Symbol)
bar(other3 => 10)
  # ^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated
bar(**{other3 => 10})


sig {params(x: Integer, y: Integer, z: String).void}
def f(x, y:, z:)
  puts x
  puts y
  puts z
end

sig {params(x: Integer, y: Integer, z: String, w: Float).void}
def g(x, y:, z:, w:)
  puts x
  puts y
  puts z
  puts w
end

shaped_hash = {y: 3, z: "hi mom"}
f(3, shaped_hash)
   # ^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated
f(3, **shaped_hash)
g(3, **shaped_hash, w: 2.0)

untyped_hash = T.let(shaped_hash, T.untyped)
f(3, untyped_hash)
   # ^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated
f(3, **untyped_hash)
g(3, **untyped_hash, w: 2.0)

untyped_values_hash = T.let(shaped_hash, T::Hash[Symbol, T.untyped])
f(3, untyped_values_hash)
   # ^^^^^^^^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated
f(3, **untyped_values_hash)
g(3, **untyped_values_hash, w: 2.0)

sig {params(x: Integer, y: Integer, kw_splat: BasicObject).void}
def h(x, y:, **kw_splat)
  puts x
  puts y
  puts kw_splat
end

untyped_values_hash = T.let({}, T::Hash[Symbol, T.untyped])
h(1, y: 2, **untyped_values_hash)
   # ^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated

sig {returns(T.untyped)}
def make_untyped; T.unsafe("hello"); end
class Tempfile_
  extend T::Sig
  sig do
    params(
      basename: T.any(String, [String, String]),
      tmpdir: T.nilable(String),
      mode: Integer,
      options: T.untyped,
    )
    .void
  end
  def initialize(basename='', tmpdir=nil, mode: 0, **options); end
end

Tempfile_.new(make_untyped)

module B
  def self.inner_foo(arg0 = [], arg1: true)
    p [arg0, arg1]

  end
  def self.outer_foo(arg0)
    # probably shouldn't
    B.inner_foo(arg0)
  end
end



# Ruby 2.7: [[], []] and also a warning
# Ruby 3.0: [{:arg1=>[]}, true]
B.inner_foo({:arg1 => []})
          # ^^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated


hash_args = {}
hash_args[:arg1] = false
# Ruby 2.7: [[], false] 
# Ruby 3.0: [{:arg1=>false}, true] 
B.inner_foo(hash_args)
          # ^^^^^^^^^ error: Keyword argument hash without `**` is deprecated

B.inner_foo([123])
array_arg = [123]
B.inner_foo(array_arg)

# Ruby 2.7: [{"foo"=>"bar"}, {"cred1"=>true, "cred2"=>false, "cred3"=>false, "cred4"=>true}, false, "x"]
# Ruby 3.0: [{"foo"=>"bar"}, {"cred1"=>true, "cred2"=>false, "cred3"=>false, "cred4"=>true}, false, "x"]
def takes_default_hash(arg0, arg1 = {}, arg2: false, arg3: "x"); end
takes_default_hash(
  {"foo" => "bar"},
  {"cred1" => true, "cred2" => false, "cred3" => false, "cred4" => true}
  # The second param here is usually dispatched as a keyword args hash
  # That's incorrect, but it's impossible to model it correctly rn.
  # We special cased this use case in calls.cc
)

# Ruby 2.7: [{"foo"=>"bar"}, {}, true, "y"] 
# Ruby 3.0: [{"foo"=>"bar"}, {:arg2=>true, :arg3=>"y"}, false, "x"]
arg1 = {:arg2 => true, :arg3 => "y"}
takes_default_hash(
  {"foo" => "bar"},
  arg1
# ^^^^ error: Keyword argument hash without `**` is deprecated
)

# Ruby 2.7: [{"foo"=>"bar"}, {"arg2"=>true, "arg3"=>"y"}, false, "x"]
# Ruby 3.0: [{"foo"=>"bar"}, {"arg2"=>true, "arg3"=>"y"}, false, "x"]
takes_default_hash(
  {"foo" => "bar"},
  {"arg2" => true, "arg3" => "y"}
)

# Ruby 2.7: [{"foo"=>"bar"}, {}, true, "y"] 
# Ruby 3.0: [{"foo"=>"bar"}, {:arg2=>true, :arg3=>"y"}, false, "x"]
takes_default_hash(
  {"foo" => "bar"},
  {arg2: true, arg3: "y"}
# ^^^^^^^^^^^^^^^^^^^^^^^ error: Keyword argument hash without `**` is deprecated
)
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


