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
