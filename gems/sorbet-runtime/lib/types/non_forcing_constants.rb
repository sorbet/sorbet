# frozen_string_literal: true
# typed: strict

module T::NonForcingConstants
  # NOTE: This method is documented on the RBI in Sorbet's payload, so that it
  # shows up in the hover/completion documentation via LSP.
  T::Sig::WithoutRuntime.sig {params(val: BasicObject, klass: String, package: T.nilable(String)).returns(T::Boolean)}
  def self.non_forcing_is_a?(val, klass, package: nil)
    method_name = "T::NonForcingConstants.non_forcing_is_a?"
    found_klass = non_forcing_lookup(klass, method_name)
    !!(found_klass && found_klass.===(val))
  end

  T::Sig::WithoutRuntime.sig {params(val: Module, klass: String, package: T.nilable(String)).returns(T::Boolean)}
  def self.non_forcing_inherits_from?(val, klass, package: nil)
    method_name = "T::NonForcingConstants.non_forcing_inherits_from?"
    if !val.is_a?(T.unsafe(Module))
      raise ArgumentError.new("The value `#{val}` provided to `#{method_name}` must be a Module or Class")
    end

    found_klass = non_forcing_lookup(klass, method_name)
    !!(found_klass && val.<(found_klass))
  end

  T::Sig::WithoutRuntime.sig {params(klass: String, method_name: String, package: T.nilable(String)).returns(T.nilable(Module))}
  private_class_method def self.non_forcing_lookup(klass, method_name, package: nil)
    # TODO(gdritter): once we have a runtime implementation of
    # packages, we'll need to actually handle the `package` argument
    # here.
    if klass.empty?
      raise ArgumentError.new("The string given to `#{method_name}` must not be empty")
    end
    current_klass = T.let(nil, T.nilable(Module))
    current_prefix = T.let(nil, T.nilable(String))

    parts = klass.split('::')
    parts.each do |part|
      if current_klass.nil?
        # First iteration
        if part != "" && package.nil?
          # if we've supplied a package, we're probably running in
          # package mode, which means absolute references are
          # meaningless
          raise ArgumentError.new("The string given to `#{method_name}` must be an absolute constant reference that starts with `::`")
        end

        current_klass = Object
        current_prefix = ''
      else
        if current_klass.autoload?(part)
          # There's an autoload registered for that constant, which means it's not
          # yet loaded. `value` can't be an instance of something not yet loaded.
          return nil
        end

        # Sorbet guarantees that the string is an absolutely resolved name.
        search_inheritance_chain = false
        if !current_klass.const_defined?(part, search_inheritance_chain)
          return nil
        end

        current_klass = current_klass.const_get(part)
        current_prefix = "#{current_prefix}::#{part}"

        if !Module.===(current_klass)
          raise ArgumentError.new("#{current_prefix} is not a class or module")
        end
      end
    end
    current_klass
  end
end
