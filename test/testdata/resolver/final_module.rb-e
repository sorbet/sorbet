# typed: false
# The fast path causes duplicate errors.
# disable-fast-path: true

class FinalClass
  extend T::Helpers
  final!
end

module FinalModule
  extend T::Helpers
  final!
end

module NonFinalModule; end

class BadInherit < FinalClass # error: `FinalClass` was declared as final and cannot be inherited by `BadInherit`
end

module BadInclude
  include NonFinalModule
  extend NonFinalModule
  include FinalModule # error: `FinalModule` was declared as final and cannot be included in `BadInclude`
end

module BadExtend
  include NonFinalModule
  extend NonFinalModule
  extend FinalModule # error: `FinalModule` was declared as final and cannot be extended in `BadExtend`
end

AliasFinalModule = FinalModule

module BadAliasInclude
  include AliasFinalModule # error: `FinalModule` was declared as final and cannot be included in `BadAliasInclude`
end

module FinalModuleWithFinalMethods
  extend T::Helpers
  final!
  extend T::Sig
  sig(:final) {void}
  def foo; end
  sig(:final) {void}
  def self.bar; end
end

module FinalModuleWithNoFinalMethods
  extend T::Helpers
  final!
  def foo; end # error: `FinalModuleWithNoFinalMethods` was declared as final but its method `foo` was not declared as final
  def self.bar; end # error: `FinalModuleWithNoFinalMethods` was declared as final but its method `bar` was not declared as final
end

class ClassWithClass
  extend T::Helpers
  final!
  class C; end
end

class ClassWithClassOkay < ClassWithClass::C; end
class ClassWithClassBad < ClassWithClass; end # error: `ClassWithClass` was declared as final and cannot be inherited by `ClassWithClassBad`

class ClassWithModule
  extend T::Helpers
  final!
  module M; end
end

class ClassWithModuleOkay; include ClassWithModule::M; end
class ClassWithModuleBad < ClassWithModule; end # error: `ClassWithModule` was declared as final and cannot be inherited by `ClassWithModuleBad`

module ModuleWithClass
  extend T::Helpers
  final!
  class C; end
end

class ModuleWithClassOkay < ModuleWithClass::C; end
class ModuleWithClassBad; include ModuleWithClass; end # error: `ModuleWithClass` was declared as final and cannot be included in `ModuleWithClassBad`

module ModuleWithModule
  extend T::Helpers
  final!
  module M; end
end

class ModuleWithModuleOkay; include ModuleWithModule::M; end
class ModuleWithModuleBad; include ModuleWithModule; end # error: `ModuleWithModule` was declared as final and cannot be included in `ModuleWithModuleBad`
