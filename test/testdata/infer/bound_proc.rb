# typed: true

class Base
  def self.before_save(method_name, **options)
  end

  def self.before_create(method_name, **options)
  end
end

module Concern
  def included(&block)
    @block = block
  end
end

module Readable
  extend Concern

  included do
    before_create :do_this
  # ^^^^^^^^^^^^^ error: Method `before_create` does not exist on `T.class_of(Readable)`
  end
end

module Writable
  extend Concern

  included do
    T.bind(self, T.any(T.class_of(Article), T.class_of(Post)))
    some_class_method :name
  # ^^^^^^^^^^^^^^^^^ error: Method `some_class_method` does not exist on `T.class_of(Article)` component of `T.any(T.class_of(Article), T.class_of(Post))`
  end
end

module Shareable
  extend Concern

  included do
    T.bind(self, T.class_of(Article))
    before_save :do_this
  end
end

class Post < Base
  include Readable
  include Writable

  before_create :run_callback, if: -> { should_run_callback? }
                                      # ^^^^^^^^^^^^^^^^^^^^ error: Method `should_run_callback?` does not exist on `T.class_of(Post)`

  def self.some_class_method(name); end
  def author; end
  def should_run_callback?; true; end

  def score
    T.bind("100", Integer)
  # ^^^^^^^^^^^^^^^^^^^^^^ error: `T.bind` can only be used with `self`
  end
end

class Article < Base
  include Writable
  include Shareable

  before_create :run_callback, if: -> { T.bind(self, Article).should_run_callback? }

  def author; end
  def should_run_callback?; true; end
end

class A
  def instance_helper; end

  f = -> do
    T.bind(self, A)

    T.reveal_type(self) # error: type: `A`

    self.instance_helper
  end

  T.reveal_type(self) # error: type: `T.class_of(A)`

  puts f
end

class B
  extend T::Sig

  sig {params(blk: T.proc.void).void}
  def self.class_helper(&blk); end

  def instance_helper; end
end

B.class_helper {
  T.bind(self, B)

  T.reveal_type(self) # error: type: `B`

  self.instance_helper
}

T.reveal_type(self) # error: type: `T.class_of(<root>)`

module N
  def helper_from_N; end
end

module M
  extend T::Sig

  def helper_from_M; end

  def main
    T.bind(self, T.all(M, N))
    T.reveal_type(self) # error: type: `T.all(M, N)`

    helper_from_M
    helper_from_N
  end
end

module ThisSelf
  extend T::Sig

  def main
    this = T.bind(self, Kernel)

    T.reveal_type(this) # error: type: `Kernel`

    this.puts
  end
end

class Rescues
  def self.takes_block(&block); end

  takes_block do
    T.bind(self, Integer)
    T.reveal_type(self) # error: type: `Integer`
  rescue
    T.reveal_type(self) # error: type: `Integer`
  end

  def foo
    T.bind(self, String)
    T.reveal_type(self) # error: type: `String`
  rescue
    T.reveal_type(self) # error: type: `String`
  ensure
    T.reveal_type(self) # error: type: `String`
  end

  def bar
    T.bind(self, String)
    T.reveal_type(self) # error: type: `String`
  rescue
    T.bind(self, Integer)
    T.reveal_type(self) # error: type: `Integer`
  ensure
    T.bind(self, Float)
    T.reveal_type(self) # error: type: `Float`
  end

  def baz
    T.reveal_type(self) # error: type: `Rescues`

    begin
      T.bind(self, String)
      T.reveal_type(self) # error: type: `String`
    rescue
      T.reveal_type(self) # error: type: `String`
    end
  end
end

class UntypedBind
  def foo
    T.reveal_type(self) # error: type: `UntypedBind`
    T.bind(self, T.untyped)
    T.reveal_type(self) # error: type: `T.untyped`
    T.bind(self, UntypedBind)
    T.reveal_type(self) # error: type: `UntypedBind`
  end
end
