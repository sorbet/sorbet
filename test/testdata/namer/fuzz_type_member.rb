# typed: true
# disable-fast-path: true
A=3 # error: Reassigning field with a value of wrong type
A=type_member # error: `type_member` cannot be used at the top-level
