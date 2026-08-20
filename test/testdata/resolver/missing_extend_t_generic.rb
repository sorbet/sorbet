# typed: true

class HasTypeMember
  Elem = type_member
#        ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(HasTypeMember)`
end

class HasTypeTemplate
  Elem = type_template
#        ^^^^^^^^^^^^^ error: Method `type_template` does not exist on `T.class_of(HasTypeTemplate)[T.attached_class (of HasTypeTemplate), T.class_of(HasTypeTemplate)::Elem]`
end

module HasAttachedClass
  has_attached_class!
# ^^^^^^^^^^^^^^^^^^^ error: Method `has_attached_class!` does not exist on `T.class_of(HasAttachedClass)`
# ^^^^^^^^^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(HasAttachedClass)`
end
