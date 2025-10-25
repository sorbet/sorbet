# typed: true

# It's okay to subclass BasicObject
class SubclassBasicObject < BasicObject; end

# It's okay to subclass Object
class SubclassObject < Object; end

# It's okay to subclass Module (the class)
class SubclassModule2 < Module; end

# It's not okay to subclass Class, but this is not actually checked.
# At one point, it was checked indirectly via checks for `has_attached_class!`,
# but those no longer check this. This code *will* raise in the Ruby VM.
class SubclassClass < Class
  # ... but we can't ENFORCE that it _never_ happens
  def example
    new
  end
end

module M; end

# It's not okay to subclass a module
  class SubclassModule1 < M; end
# ^^^^^^^^^^^^^^^^^^^^^^^^^ error: The super class `M` of `SubclassModule1` does not derive from `Class`
