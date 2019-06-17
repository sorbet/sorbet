# typed: true

class IS < T::InexactStruct; end
class Yep < IS; end

class S < T::Struct; end
class Nope < S; end # error: Subclassing `S` is not allowed
