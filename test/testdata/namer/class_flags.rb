# typed: true

class AbstractClass
  extend T::Helpers
  abstract!
end

module Interface
  extend T::Helpers
  interface!
end

class FinalClass
  extend T::Helpers
  final!
end

class SealedClass
  extend T::Helpers
  sealed!
end

class SeveralFlags
  extend T::Helpers
  abstract!
  sealed!
end
