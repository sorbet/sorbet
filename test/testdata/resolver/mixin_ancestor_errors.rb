# typed: true
# check-out-of-order-constant-references: true

class A
  include DoesNotExist1
  #       ^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist1`
  extend DoesNotExist2
  #      ^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist2`
  prepend DoesNotExist3
  #       ^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist3`
end

module M
  include DoesNotExist4
  #       ^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist4`
end

class B
  class << self
    include DoesNotExist5
    #       ^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist5`
  end
end

class C < DoesNotExist6
#         ^^^^^^^^^^^^^ error: Unable to resolve constant `DoesNotExist6`
end

module Holder
  module Priv; end
  private_constant :Priv
end

class D
  include Holder::Priv
  #       ^^^^^^^^^^^^ error: Non-private reference to private constant `Holder::Priv` referenced
  include Later
  #       ^^^^^ error: `Later` referenced before it is defined
end

module Later; end
