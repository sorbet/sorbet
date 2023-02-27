# frozen_string_literal: true
# typed: true

# A marking class for when methods return void.
# Should never appear in types directly.
class T::Private::Types::Void < T::Types::Base
  ERROR_MESSAGE = "Validation is being done on an `Void`. Please report this bug at https://github.com/sorbet/sorbet/issues"

  class VoidSingleton < BasicObject
    def inspect
      "T::Private::Types::Void::VOID"
    end

    ALLOWED_KERNEL_METHODS = [:object_id, :class, :respond_to?, :eql?]

    if defined?(::PP::ObjectMixin)
      include ::PP::ObjectMixin
      alias_method :pretty_inspect, :inspect
    end

    if defined?(::AwesomePrint)
      def ai(options = {})
        ::Kernel.instance_method(:ai).bind_call(self)
      end

      def method
        ::Kernel.instance_method(:method).bind_call(self)
      end
    end

    def method_missing(method_name, *args)
      if ALLOWED_KERNEL_METHODS.include?(method_name)
        ::Kernel.instance_method(method_name).bind_call(self, *args)
      else
        ::Kernel.raise(::TypeError, "Attempted to call ##{method_name} on the result of a void-returning method.")
      end
    end

    freeze
  end

  # The actual return value of `.void` methods.
  VOID = VoidSingleton.new
  Kernel.instance_method(:freeze).bind_call(VOID)

  # overrides Base
  def name
    "<VOID>"
  end

  # overrides Base
  def valid?(obj)
    raise ERROR_MESSAGE
  end

  # overrides Base
  private def subtype_of_single?(other)
    raise ERROR_MESSAGE
  end
end
