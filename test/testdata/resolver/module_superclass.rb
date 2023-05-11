# typed: true
# disable-fast-path: true

# It's okay to subclass BasicObject
class SubclassBasicObject < BasicObject; end

# It's okay to subclass Object
class SubclassObject < Object; end

# It's okay to subclass Module (the class)
class SubclassModule2 < Module; end

# It's not okay to subclass Class
class SubclassClass < Class; end # error: `SubclassClass` is a subclass of `Class` which is not allowed

module M; end

# It's not okay to subclass a module
  class SubclassModule1 < M; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: The super class `M` of `SubclassModule1` does not derive from `Class`
