# frozen_string_literal: true

class Module < Object
  # Makes a method package-private.
  #
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # class SimpleSingleton  # Not thread safe
  #   private_class_method :new
  #   def SimpleSingleton.create(*args, &block)
  #     @me = new(*args, &block) if ! @me
  #     @me
  #   end
  # end
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def package_private(*arg0); end

  # Makes class methods package-private.
  #
  # [`String`](https://docs.ruby-lang.org/en/2.7.0/String.html) arguments are
  # converted to symbols.
  #
  # ```ruby
  # class SimpleSingleton  # Not thread safe
  #   private_class_method :new
  #   def SimpleSingleton.create(*args, &block)
  #     @me = new(*args, &block) if ! @me
  #     @me
  #   end
  # end
  # ```
  sig do
    params(
        arg0: T.any(Symbol, String),
    )
    .returns(T.self_type)
  end
  def package_private_class_method(*arg0); end
end
