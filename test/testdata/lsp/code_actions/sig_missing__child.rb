# typed: strict
# selective-apply-code-action: quickfix

require_relative './sig_missing__parent.rb'

class FooChild < Foo
  def self.bar
# ^^^^^^^^^^^^ error: This function does not have a `sig`
# ^^^^^^^^^^^^ apply-code-action: [A] Add `sig {override.void}`
  end
end
