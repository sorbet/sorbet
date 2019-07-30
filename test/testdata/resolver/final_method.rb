# typed: false
# disable-fast-path: true

class Redefine
  extend T::Sig

  def foo; end

  def self.bar; end

  sig(:final) {void}
  def foo; end

  sig(:final) {void}
  def self.bar; end

  sig(:final) {void}
  def foo; end # TODO should be error: `A#foo` was declared as final and cannot be redefined

  sig(:final) {void}
  def self.bar; end # TODO should be error: `A.foo` was declared as final and cannot be redefined

  sig {void}
  def foo; end # TODO should be error: `A#foo` was declared as final and cannot be redefined

  sig {void}
  def self.bar; end # TODO should be error: `A.foo` was declared as final and cannot be redefined

  def foo; end # TODO should be error: `A#foo` was declared as final and cannot be redefined

  def self.bar; end # TODO should be error: `A.foo` was declared as final and cannot be redefined
end

class C
  extend T::Sig
  sig(:final) {void}
  def foo; end
  sig(:final) {void}
  def self.bar; end
end

class OverrideInherit < C
  def foo; end # error: `C#foo` was declared as final and cannot be overridden by `OverrideInherit#foo`
  def self.bar; end # error: `C.bar` was declared as final and cannot be overridden by `OverrideInherit.bar`
end

module M1
  extend T::Sig
  sig(:final) {void}
  def foo; end
end

module M2
  extend T::Sig
  sig(:final) {void}
  def foo; end
end

class OverrideInclude
  include M1
  def foo; end # error: `M1#foo` was declared as final and cannot be overridden by `OverrideInclude#foo`
end

class OverrideExtend
  extend M1
  def self.foo; end # error: `M1#foo` was declared as final and cannot be overridden by `OverrideExtend.foo`
end

class OverrideDoubleInclude
  include M1
  include M2 # TODO should be error: `M1#foo` was declared as final and cannot be overridden by `M2#foo`
end

class OverrideDoubleExtend
  extend M1
  extend M2 # TODO should be error: `M1#foo` was declared as final and cannot be overridden by `M2#foo`
end

module Step1; include M1; end
module Step2; include Step1; end
module Step3; include Step2; end
module OverrideManySteps
  include Step3
  def foo; end # error: `M1#foo` was declared as final and cannot be overridden by `OverrideManySteps#foo`
end

module IncludeAgain
  include M1
  include M1
end

module ExtendAgain
  extend M1
  extend M1
end
