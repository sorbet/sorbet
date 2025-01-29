# typed: true
attr"foo bar" # error: Bad attribute name `foo bar`

attr "" # error: Attribute names must be non-empty
attr_accessor "" # error: Attribute names must be non-empty
attr_reader "" # error: Attribute names must be non-empty
attr_writer "" # error: Attribute names must be non-empty
