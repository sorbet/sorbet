# frozen_string_literal: true
# typed: true

module T::Private::Methods
  Declaration = Struct.new(
    :mod,
    :params,
    :returns,
    :bind,
    :mode,
    :checked,
    :finalized,
    :on_failure,
    :override_allow_incompatible,
    :type_parameters,
    :raw,
    :final
  )

  class DeclBuilder
    # The signature declaration the builder is composing (class `Declaration`)
    attr_accessor :decl

    class BuilderError < StandardError; end

    def raise_after_reset(error)
      # Because some signatures raise before even running the block, we need to cleanup after ourselves or else the other
      # tests think there's an active declaration without a corresponding method definition
      T::Private::DeclState.current.reset!
      raise error
    end

    private def check_live!
      if decl.finalized
        raise_after_reset BuilderError.new("You can't modify a signature declaration after it has been used.")
      end
    end

    private def check_sig_block_is_unset!
      if T::Private::DeclState.current.active_declaration&.blk
        raise_after_reset BuilderError.new(
          "Cannot add more signature statements after the declaration block."
        )
      end
    end

    # Verify if we're trying to invoke the method outside of a signature block. Notice that we need to check if it's a
    # proc, because this is valid and would lead to a false positive: `T.type_alias { T.proc.params(a: Integer).void }`
    private def check_running_inside_block!(method_name)
      unless @inside_sig_block || decl.mod == T::Private::Methods::PROC_TYPE
        T::Private::DeclState.current.reset!
        raise_after_reset BuilderError.new(
          "Can't invoke #{method_name} outside of a signature declaration block"
        )
      end
    end

    def initialize(mod, raw)
      @decl = Declaration.new(
        mod,
        ARG_NOT_PROVIDED, # params
        ARG_NOT_PROVIDED, # returns
        ARG_NOT_PROVIDED, # bind
        Modes.standard, # mode
        ARG_NOT_PROVIDED, # checked
        false, # finalized
        ARG_NOT_PROVIDED, # on_failure
        nil, # override_allow_incompatible
        ARG_NOT_PROVIDED, # type_parameters
        raw,
        ARG_NOT_PROVIDED, # final
      )
      @inside_sig_block = false
    end

    def run!(&block)
      @inside_sig_block = true
      instance_exec(&block)
      finalize!
      self
    end

    def params(*unused_positional_params, **params)
      check_live!
      check_running_inside_block!(__method__)

      if !decl.params.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .params twice")
      end

      if unused_positional_params.any?
        some_or_only = params.any? ? "some" : "only"
        raise_after_reset BuilderError.new(<<~MSG)
          'params' was called with #{some_or_only} positional arguments, but it needs to be called with keyword arguments.
          The keyword arguments' keys must match the name and order of the method's parameters.
        MSG
      end

      if params.empty?
        raise_after_reset BuilderError.new(<<~MSG)
          'params' was called without any arguments, but it needs to be called with keyword arguments.
          The keyword arguments' keys must match the name and order of the method's parameters.

          Omit 'params' entirely for methods with no parameters.
        MSG
      end

      decl.params = params

      self
    end

    def returns(type)
      check_live!
      check_running_inside_block!(__method__)

      if decl.returns.is_a?(T::Private::Types::Void)
        raise_after_reset BuilderError.new("You can't call .returns after calling .void.")
      end
      if !decl.returns.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .returns multiple times in a signature.")
      end

      decl.returns = type

      self
    end

    def void
      check_live!
      check_running_inside_block!(__method__)

      if !decl.returns.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .void after calling .returns.")
      end

      decl.returns = T::Private::Types::Void::Private::INSTANCE

      self
    end

    def bind(type)
      check_live!
      check_running_inside_block!(__method__)

      if !decl.bind.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .bind multiple times in a signature.")
      end

      decl.bind = type

      self
    end

    def checked(level)
      check_live!
      check_running_inside_block!(__method__)

      if !decl.checked.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .checked multiple times in a signature.")
      end
      if (level == :never || level == :compiled) && !decl.on_failure.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't use .checked(:#{level}) with .on_failure because .on_failure will have no effect.")
      end
      if !T::Private::RuntimeLevels::LEVELS.include?(level)
        raise_after_reset BuilderError.new("Invalid `checked` level '#{level}'. Use one of: #{T::Private::RuntimeLevels::LEVELS}.")
      end

      decl.checked = level

      self
    end

    def on_failure(*args)
      check_live!
      check_running_inside_block!(__method__)

      if !decl.on_failure.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .on_failure multiple times in a signature.")
      end
      if decl.checked == :never || decl.checked == :compiled
        raise_after_reset BuilderError.new("You can't use .on_failure with .checked(:#{decl.checked}) because .on_failure will have no effect.")
      end

      decl.on_failure = args

      self
    end

    def abstract(&blk)
      check_live!

      case decl.mode
      when Modes.standard
        decl.mode = Modes.abstract
      when Modes.abstract
        raise_after_reset BuilderError.new(".abstract cannot be repeated in a single signature")
      else
        raise_after_reset BuilderError.new("`.abstract` cannot be combined with `.override` or `.overridable`.")
      end

      check_sig_block_is_unset!

      if blk
        T::Private::DeclState.current.active_declaration.blk = blk
      end

      self
    end

    def final(&blk)
      check_live!

      if !decl.final.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .final multiple times in a signature.")
      end

      if @inside_sig_block
        raise_after_reset BuilderError.new(
          "Unlike other sig annotations, the `final` annotation must remain outside the sig block, " \
          "using either `sig(:final) {...}` or `sig.final {...}`, not `sig {final. ...}"
        )
      end

      raise_after_reset BuilderError.new(".final cannot be repeated in a single signature") if final?

      decl.final = true

      check_sig_block_is_unset!

      if blk
        T::Private::DeclState.current.active_declaration.blk = blk
      end

      self
    end

    def final?
      !decl.final.equal?(ARG_NOT_PROVIDED) && decl.final
    end

    def override(allow_incompatible: false, &blk)
      check_live!

      case decl.mode
      when Modes.standard
        decl.mode = Modes.override
        decl.override_allow_incompatible = allow_incompatible
      when Modes.override, Modes.overridable_override
        raise_after_reset BuilderError.new(".override cannot be repeated in a single signature")
      when Modes.overridable
        decl.mode = Modes.overridable_override
      else
        raise_after_reset BuilderError.new("`.override` cannot be combined with `.abstract`.")
      end

      check_sig_block_is_unset!

      if blk
        T::Private::DeclState.current.active_declaration.blk = blk
      end

      self
    end

    def overridable(&blk)
      check_live!

      case decl.mode
      when Modes.abstract
        raise_after_reset BuilderError.new("`.overridable` cannot be combined with `.#{decl.mode}`")
      when Modes.override
        decl.mode = Modes.overridable_override
      when Modes.standard
        decl.mode = Modes.overridable
      when Modes.overridable, Modes.overridable_override
        raise_after_reset BuilderError.new(".overridable cannot be repeated in a single signature")
      end

      check_sig_block_is_unset!

      if blk
        T::Private::DeclState.current.active_declaration.blk = blk
      end

      self
    end

    # Declares valid type paramaters which can be used with `T.type_parameter` in
    # this `sig`.
    #
    # This is used for generic methods. Example usage:
    #
    #  sig do
    #    type_parameters(:U)
    #    .params(blk: T.proc.params(arg0: Elem).returns(T.type_parameter(:U)))
    #    .returns(T::Array[T.type_parameter(:U)])
    #  end
    #  def map(&blk); end
    def type_parameters(*names)
      check_live!
      check_running_inside_block!(__method__)

      names.each do |name|
        raise_after_reset BuilderError.new("not a symbol: #{name}") unless name.is_a?(Symbol)
      end

      if !decl.type_parameters.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You can't call .type_parameters multiple times in a signature.")
      end

      decl.type_parameters = names

      self
    end

    def finalize!
      check_live!

      if decl.returns.equal?(ARG_NOT_PROVIDED)
        raise_after_reset BuilderError.new("You must provide a return type; use the `.returns` or `.void` builder methods.")
      end

      if decl.bind.equal?(ARG_NOT_PROVIDED)
        decl.bind = nil
      end
      if decl.checked.equal?(ARG_NOT_PROVIDED)
        default_checked_level = T::Private::RuntimeLevels.default_checked_level
        if (default_checked_level == :never || default_checked_level == :compiled) && !decl.on_failure.equal?(ARG_NOT_PROVIDED)
          raise_after_reset BuilderError.new("To use .on_failure you must additionally call .checked(:tests) or .checked(:always), otherwise, the .on_failure has no effect.")
        end
        decl.checked = default_checked_level
      end
      if decl.on_failure.equal?(ARG_NOT_PROVIDED)
        decl.on_failure = nil
      end
      if decl.params.equal?(ARG_NOT_PROVIDED)
        decl.params = FROZEN_HASH
      end
      if decl.type_parameters.equal?(ARG_NOT_PROVIDED)
        decl.type_parameters = FROZEN_HASH
      end

      decl.finalized = true

      self
    end

    FROZEN_HASH = {}.freeze
  end
end
