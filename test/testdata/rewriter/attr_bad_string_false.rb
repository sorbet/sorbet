# typed: false
attr :'foo-bar' # error: Bad attribute name `foo-bar`
attr_reader :'left-right' # error: Bad attribute name `left-right`
attr_writer :'baz-qux' # error: Bad attribute name `baz-qux`
attr_accessor :'a-b' # error: Bad attribute name `a-b`
attr :"" # error: Attribute names must be non-empty
attr 10 # error: Argument to `attr` must be a Symbol or String
