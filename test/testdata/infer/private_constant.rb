# typed: true

module Foo
  A_PUBLIC_CONST = 'public'
  A_PRIVATE_CONST = 'private'
  private_constant :A_PRIVATE_CONST

  ANOTHER_PRIVATE_CONST = true
  private_constant 'ANOTHER_PRIVATE_CONST'

  class PrivateClass; end
  private_constant :PrivateClass

  class AnotherPrivateClass; end
  private_constant 'AnotherPrivateClass'

  module PrivateModule; end
  private_constant :PrivateModule

  module AnotherPrivateModule; end
  private_constant 'AnotherPrivateModule'
end
