# frozen_string_literal: true
# typed: true

# Cut down version of Chalk::Tools::ClassUtils with only :replace_method functionality.
# Extracted to a separate namespace so the type system can be used standalone.
#
# Note: the functionality to "restore" a method was removed, because it is no
# longer being used by sorbet-runtime. Restoring a method requires care--if you
# need to reintroduce this functionality, consult the git history for how to do
# it safely.
module T::Private::ClassUtils
  # Calling a method on `mod` caches the lookup in `mod`'s own call cache table, which
  # lives as long as `mod` does. sorbet-runtime reflects on every module that defines a
  # method with a sig, so calling these through `bind_call` avoids leaving a cache entry
  # behind in each of them.
  PUBLIC_METHOD_DEFINED = Module.instance_method(:public_method_defined?)
  PROTECTED_METHOD_DEFINED = Module.instance_method(:protected_method_defined?)
  PRIVATE_METHOD_DEFINED = Module.instance_method(:private_method_defined?)
  MODULE_EXEC = Module.instance_method(:module_exec)
  DEFINE_METHOD = Module.instance_method(:define_method)
  INSTANCE_METHOD = Module.instance_method(:instance_method)
  RUBY2_KEYWORDS = Module.instance_method(:ruby2_keywords)
  VISIBILITY_METHODS = {
    public: Module.instance_method(:public),
    protected: Module.instance_method(:protected),
    private: Module.instance_method(:private),
  }.freeze
  EQUAL = BasicObject.instance_method(:equal?)
  private_constant :PUBLIC_METHOD_DEFINED, :PROTECTED_METHOD_DEFINED, :PRIVATE_METHOD_DEFINED,
                   :MODULE_EXEC, :DEFINE_METHOD, :INSTANCE_METHOD, :RUBY2_KEYWORDS,
                   :VISIBILITY_METHODS, :EQUAL

  # `name` must be an instance method (for class methods, pass in mod.singleton_class)
  def self.visibility_method_name(mod, name)
    if PUBLIC_METHOD_DEFINED.bind_call(mod, name)
      :public
    elsif PROTECTED_METHOD_DEFINED.bind_call(mod, name)
      :protected
    elsif PRIVATE_METHOD_DEFINED.bind_call(mod, name)
      :private
    else
      # Raises a NameError formatted like the Ruby VM would (the exact text formatting
      # of these errors changed across Ruby VM versions, in ways that would sometimes
      # cause tests to fail if they were dependent on hard coding errors).
      mod.method(name)
      # This is just to prove to Sorbet that this method raises.
      # This line should be unreachable in practice due to the above call.
      raise NameError.new("undefined method `#{name}' for class `#{mod}'")
    end
  end

  def self.def_with_visibility(mod, name, visibility, method=nil, &block)
    MODULE_EXEC.bind_call(mod) do
      # Start a visibility (public/protected/private) region, so that
      # all of the method redefinitions happen with the right visibility
      # from the beginning. This ensures that any other code that is
      # triggered by `method_added`, sees the redefined method with the
      # right visibility.
      VISIBILITY_METHODS.fetch(visibility).bind_call(mod)

      if method
        DEFINE_METHOD.bind_call(mod, name, method)
      else
        DEFINE_METHOD.bind_call(mod, name, &block)
      end

      if block && block.arity < 0
        # Fetch .parameters once: each call builds a fresh array.
        parameters = INSTANCE_METHOD.bind_call(mod, name).parameters
        has_rest = parameters.any? { |kind, _| kind == :rest }
        has_keywords = parameters.any? { |kind, _| kind == :keyrest || kind == :keyreq || kind == :key }
        RUBY2_KEYWORDS.bind_call(mod, name) if has_rest && !has_keywords
      end
    end
  end

  # Replaces a method, either by overwriting it (if it is defined directly on
  # `mod`) or by overriding it (if it is defined by one of mod's ancestors).
  #
  # Takes the `original_method` as a parameter, so it does not return anything.
  #
  # Can also avoid `T.let` pinning errors by letting the caller pre-compute the
  # `original_method`, so it knows that it will always be defined (because it
  # doesn't know that the block will always run once)
  #
  # Does not share code with `replace_method_with_handle`, for performance (do
  # not want to increase the call stack, as this is a very sensitive code path).
  def self.replace_method(original_method, mod, name, &blk)
    original_visibility = visibility_method_name(mod, name)
    original_owner = original_method.owner

    # In the common case the method is owned by `mod` itself, in which case the loop below
    # would always `break` before the raise could match (`mod` precedes any non-prepended
    # ancestor in its own ancestor chain), so skip computing `mod.ancestors` entirely.
    if !EQUAL.bind_call(original_owner, mod)
      mod.ancestors.each do |ancestor|
        break if ancestor == mod
        if ancestor == original_owner
          # If we get here, that means the method we're trying to replace exists on a *prepended*
          # mixin, which means in order to supersede it, we'd need to create a method on a new
          # module that we'd prepend before `ancestor`. The problem with that approach is there'd
          # be no way to remove that new module after prepending it, so we'd be left with these
          # empty anonymous modules in the ancestor chain after calling `restore`.
          #
          # That's not necessarily a deal breaker, but for now, we're keeping it as unsupported.
          raise "You're trying to replace `#{name}` on `#{mod}`, but that method exists in a " \
                "prepended module (#{ancestor}), which we don't currently support."
        end
      end
    end

    T::Configuration.without_ruby_warnings do
      T::Private::DeclState.current.without_on_method_added do
        def_with_visibility(mod, name, original_visibility, &blk)
      end
    end

    nil
  end
end
