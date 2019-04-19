# frozen_string_literal: true
# typed: true

module T::Private::Methods
  Declaration = Struct.new(:mod, :params, :returns, :bind, :mode, :checked, :finalized, :soft_notify, :override_allow_incompatible, :type_parameters, :generated)

  class SigBuilder
    attr_reader :decl

    class SigBuilderError < StandardError; end

    private def check_live!
      if decl.finalized
        raise SigBuilderError.new("You can't modify a signature declaration after it has been used.")
      end
    end

    def initialize(mod)
      # TODO RUBYPLAT-1278 - with ruby 2.5, use kwargs here
      @decl = Declaration.new(
        mod,
        ARG_NOT_PROVIDED, # params
        ARG_NOT_PROVIDED, # returns
        ARG_NOT_PROVIDED, # bind
        Modes.standard, # mode
        ARG_NOT_PROVIDED, # checked
        false, # finalized
        ARG_NOT_PROVIDED, # soft_notify
        nil, # override_allow_incompatible
        ARG_NOT_PROVIDED, # type_parameters
        ARG_NOT_PROVIDED, # generated
      )
    end

    def params(params)
      check_live!
      if !decl.params.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .params twice")
      end

      decl.params = params

      self
    end

    def returns(type)
      check_live!
      if decl.returns.is_a?(T::Private::Types::Void)
        raise SigBuilderError.new("You can't call .returns after calling .void.")
      end
      if !decl.returns.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .returns multiple times in a signature.")
      end

      decl.returns = type

      self
    end

    def void
      check_live!
      if !decl.returns.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .void after calling .returns.")
      end

      decl.returns = T::Private::Types::Void.new

      self
    end

    def bind(type)
      check_live!
      if !decl.bind.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .bind multiple times in a signature.")
      end

      decl.bind = type

      self
    end

    def checked(level)
      check_live!

      if !decl.checked.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .checked multiple times in a signature.")
      end
      if !decl.soft_notify.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't use .checked with .soft.")
      end
      if !decl.generated.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't use .checked with .generated.")
      end
      if !T::Private::RuntimeLevels::LEVELS.include?(level)
        raise SigBuilderError.new("Invalid `checked` level '#{level}'. Use one of: #{T::Private::RuntimeLevels::LEVELS}.")
      end

      decl.checked = level

      self
    end

    def soft(notify:)
      check_live!

      if !decl.soft_notify.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .soft multiple times in a signature.")
      end
      if !decl.checked.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't use .soft with .checked.")
      end
      if !decl.generated.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't use .soft with .generated.")
      end

      # TODO consider validating that :notify is a project that sentry knows about,
      # as per https://git.corp.stripe.com/stripe-internal/pay-server/blob/master/lib/event/job/sentry_job.rb#L125
      if !notify || notify == ''
        raise SigBuilderError.new("You can't provide an empty notify to .soft().")
      end

      decl.soft_notify = notify

      self
    end

    def generated
      check_live!

      if !decl.generated.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .generated multiple times in a signature.")
      end
      if !decl.checked.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't use .generated with .checked.")
      end
      if !decl.soft_notify.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't use .generated with .soft.")
      end

      decl.generated = true

      self
    end

    def abstract
      check_live!

      case decl.mode
      when Modes.standard
        decl.mode = Modes.abstract
      when Modes.abstract
        raise SigBuilderError.new(".abstract cannot be repeated in a single signature")
      else
        raise SigBuilderError.new("`.abstract` cannot be combined with any of `.override`, `.implementation`, or "\
              "`.overridable`.")
      end

      self
    end

    def override(allow_incompatible: false)
      check_live!

      case decl.mode
      when Modes.standard
        decl.mode = Modes.override
        decl.override_allow_incompatible = allow_incompatible
      when Modes.override
        raise SigBuilderError.new(".override cannot be repeated in a single signature")
      else
        raise SigBuilderError.new("`.override` cannot be combined with any of `.abstract`, `.implementation`, or "\
              "`.overridable`.")
      end

      self
    end

    def overridable
      check_live!

      case decl.mode
      when Modes.abstract, Modes.override
        raise SigBuilderError.new("`.overridable` cannot be combined with `.#{decl.mode}`")
      when Modes.standard
        decl.mode = Modes.overridable
      when Modes.implementation
        decl.mode = Modes.overridable_implementation
      when Modes.overridable, Modes.overridable_implementation
        raise SigBuilderError.new(".overridable cannot be repeated in a single signature")
      end

      self
    end

    def implementation
      check_live!

      case decl.mode
      when Modes.abstract, Modes.override
        raise SigBuilderError.new("`.implementation` cannot be combined with `.#{decl.mode}`")
      when Modes.standard
        decl.mode = Modes.implementation
      when Modes.overridable
        decl.mode = Modes.overridable_implementation
      when Modes.implementation, Modes.overridable_implementation
        raise SigBuilderError.new(".implementation cannot be repeated in a single signature")
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

      names.each do |name|
        raise SigBuilderError.new("not a symbol: #{name}") unless name.is_a?(Symbol)
        name.is_a?(Symbol)
      end

      if !decl.type_parameters.equal?(ARG_NOT_PROVIDED)
        raise SigBuilderError.new("You can't call .type_parameters multiple times in a signature.")
      end

      decl.type_parameters = names

      self
    end

    def finalize!
      check_live!

      if decl.bind.equal?(ARG_NOT_PROVIDED)
        decl.bind = nil
      end
      if decl.checked.equal?(ARG_NOT_PROVIDED)
        decl.checked = :always
      end
      if decl.soft_notify.equal?(ARG_NOT_PROVIDED)
        decl.soft_notify = nil
      end
      if decl.generated.equal?(ARG_NOT_PROVIDED)
        decl.generated = false
      end
      if decl.params.equal?(ARG_NOT_PROVIDED)
        decl.params = {}
      end
      if decl.type_parameters.equal?(ARG_NOT_PROVIDED)
        decl.type_parameters = {}
      end

      decl.finalized = true

      self
    end
  end
end
