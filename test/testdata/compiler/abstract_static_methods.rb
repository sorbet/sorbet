# frozen_string_literal: true
# typed: true
# compiled: true
# run_filecheck: INITIAL

class IFoo
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def self.foo; end
end

# INITIAL{LITERAL}-LABEL: define internal i64 @"func_IFoo.14<static-init>
# INITIAL: call void @{{sorbet_defineMethodSingleton.*@func_IFoo.3foo,}}
# INITIAL{LITERAL}: }

class Foo < IFoo

  sig {override.void}
  def self.foo
    Kernel.p "Override!"
  end
end

p Foo.foo
