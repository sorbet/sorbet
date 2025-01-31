# typed: true
attr"foo bar" # error: Bad attribute name `foo bar`

attr "" # error: Attribute names must be non-empty
attr_accessor "" # error: Attribute names must be non-empty
attr_reader "" # error: Attribute names must be non-empty
attr_writer "" # error: Attribute names must be non-empty

attr :"" # error: Attribute names must be non-empty
attr_accessor :"" # error: Attribute names must be non-empty
attr_reader :"" # error: Attribute names must be non-empty
attr_writer :"" # error: Attribute names must be non-empty

attr 10
   # ^^ error: Argument to `attr` must be a Symbol or String
   # ^^ error: Expected `T.any(Symbol, String)`
attr_accessor nil
            # ^^^ error: Argument to `attr_accessor` must be a Symbol or String
            # ^^^ error: Expected `T.any(Symbol, String)`

