# frozen_string_literal: true
# typed: true

module T::Private::Methods::Modes
  def self.standard; 'standard'; end
  def self.abstract; 'abstract'; end
  def self.overridable; 'overridable'; end
  def self.implementation; 'implementation'; end
  def self.override; 'override'; end
  def self.overridable_implementation; 'overridable_implementation'; end
  def self.untyped; 'untyped'; end
  MODES = [self.standard, self.abstract, self.overridable, self.implementation, self.override, self.overridable_implementation, self.untyped]

  IMPLEMENT_MODES = [self.implementation, self.overridable_implementation]
  OVERRIDABLE_MODES = [self.override, self.overridable, self.overridable_implementation, self.untyped]
  OVERRIDE_MODES = [self.override]
  NON_OVERRIDE_MODES = MODES - OVERRIDE_MODES - IMPLEMENT_MODES
end
