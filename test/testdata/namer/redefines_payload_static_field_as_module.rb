# typed: false

module CSV::DEFAULT_OPTIONS::Foo # error: Redefining constant `DEFAULT_OPTIONS` as a class or module
  DoesNotExist # error: Unable to resolve constant `DoesNotExist`
end
