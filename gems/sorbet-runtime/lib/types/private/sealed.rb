# frozen_string_literal: true
# typed: false

module T::Private::Sealed
  module NoInherit
    def inherited(other)
      super
      this_line = Kernel.caller.find {|line| !line.match(/in `inherited'$/)}
      T::Private::Sealed.validate_inheritance(this_line, self, 'inherited')
    end
  end

  module NoIncludeExtend
    def included(other)
      super
      this_line = Kernel.caller.find {|line| !line.match(/in `included'$/)}
      T::Private::Sealed.validate_inheritance(this_line, self, 'included')
    end

    def extended(other)
      super
      this_line = Kernel.caller.find {|line| !line.match(/in `extended'$/)}
      T::Private::Sealed.validate_inheritance(this_line, self, 'extended')
    end
  end

  def self.declare(mod, decl_file)
    if !mod.is_a?(Module)
      raise "#{mod} is not a class or module and cannot be declared `sealed!`"
    end
    if sealed_module?(mod)
      raise "#{mod} was already declared `sealed!` and cannot be re-declared `sealed!`"
    end
    if T::Private::Final.final_module?(mod)
      raise "#{mod} was already declared `final!` and cannot be declared `sealed!`"
    end
    mod.extend(mod.is_a?(Class) ? NoInherit : NoIncludeExtend)
    if !decl_file
      raise "Couldn't determine declaration file for sealed class."
    end
    mod.instance_variable_set(:@sorbet_sealed_module_decl_file, decl_file)
  end

  def self.sealed_module?(mod)
    mod.instance_variable_defined?(:@sorbet_sealed_module_decl_file)
  end

  def self.validate_inheritance(this_line, parent, verb)
    this_file = this_line&.split(':').first
    decl_file = parent.instance_variable_get(:@sorbet_sealed_module_decl_file)

    if !this_file || !decl_file
      raise "Couldn't determine enough file information for checking sealed modules"
    end

    if !this_file.start_with?(decl_file)
      whitelist = T::Configuration.sealed_violation_whitelist
      if whitelist != nil && whitelist.any? {|pattern| this_file =~ pattern}
        return
      end

      raise "#{parent} was declared sealed and can only be #{verb} in #{decl_file}, not #{this_file}"
    end
  end
end
