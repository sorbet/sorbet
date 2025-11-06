# typed: strict
# disable-fast-path: true

class ReplacedMethods < Module # error: `has_attached_class!` declared by parent `Module` must be re-declared in `ReplacedMethods`
  extend T::Sig

  sig { returns(Symbol) }
  attr_accessor :prop

  sig do
    params(
      prop: Symbol,
      blk: T.proc.bind(T.self_type).params(arg0: T::Module[T.anything]).returns(BasicObject)
    )
      .void
  end
  def initialize(prop, &blk)
    @prop = prop
    super(&blk)
    include
  end
end

module Tokenizable::Mixin::ClassMethods
  extend T::Sig, T::Helpers
  abstract!

  sig { abstract.params(arg0: T::Module[T.anything]).returns(T.self_type) }
  def include(arg0); end

  sig { params(method_name: Symbol).returns(Proc) }
  private def __tokenizable__hide_method!(method_name)
    T.bind(self, T::Module[T.anything])

    unless method_defined?(method_name, true)
      return -> (_, *args) { args.first }
    end

    hidden_method_name = "__tokenizable__hidden_#{method_name}".to_sym
    alias_method(hidden_method_name, method_name)
    private(hidden_method_name)
    remove_method(method_name)

    -> (receiver, *args) do
      receiver.send(hidden_method_name, *args)
    end
  end

  sig { params(name: Symbol).void }
  def tokenize(name)
    hidden_getter = __tokenizable__hide_method!(name)
    hidden_setter = __tokenizable__hide_method!("#{name}=".to_sym)

    replaced_module = ReplacedMethods.new(name) do
      define_method(name) do
        begin
          super()
        rescue NoMethodError, NotImplementedError
          hidden_getter.call(self)
        end
      end

      define_method("#{name}=".to_sym) do |value|
        begin
          super(value)
        rescue NoMethodError, NotImplementedError
          hidden_setter.call(self, value)
        end
      end
    end

    self.include(replaced_module)
  end
end
