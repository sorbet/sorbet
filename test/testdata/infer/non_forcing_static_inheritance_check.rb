# typed: true

x = T.unsafe(nil)

class Base
end

class Derived < Base
end

class Derived2 < Derived
end

class Derived3 < Derived
end

NotClass = 1

T::NonForcingConstants.static_inheritance_check # error: Not enough arguments

str = "foo"
T::NonForcingConstants.static_inheritance_check(str, Base)
#                                               ^^^  error: only accepts string literals

T::NonForcingConstants.static_inheritance_check("X", Base) # error: must be an absolute constant reference

T::NonForcingConstants.static_inheritance_check("::NotFound", Base)
#                                               ^^^^^^^^^^^^  error: Unable to resolve constant `::NotFound`

T::NonForcingConstants.static_inheritance_check("::NotClass", Derived2)
#                                               ^^^^^^^^^^^^  error: The string given to `T::NonForcingConstants.static_inheritance_check` must resolve to a class or module

T::NonForcingConstants.static_inheritance_check("::Derived3", Derived2)
#                                               ^^^^^^^^^^^^  error: Derived3 does not inherit from Derived2

# No errors below
T::NonForcingConstants.static_inheritance_check("::Derived", Base)
T::NonForcingConstants.static_inheritance_check("::Derived2", Derived)
T::NonForcingConstants.static_inheritance_check("::Derived2", Base)
