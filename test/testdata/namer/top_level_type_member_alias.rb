# typed: strict

# These snippets just need to not crash Sorbet

A = type_member
#   ^^^^^^^^^^^ error: `type_member` cannot be used at the top-level
A = T.type_alias {Integer}

::B = type_member
#     ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(<root>)`
::B = T.type_alias {Integer}

C = T.type_alias {Integer}
C = type_member
#   ^^^^^^^^^^^ error: `type_member` cannot be used at the top-level

::D = T.type_alias {Integer}
::D = type_member
#     ^^^^^^^^^^^ error: Method `type_member` does not exist on `T.class_of(<root>)`
