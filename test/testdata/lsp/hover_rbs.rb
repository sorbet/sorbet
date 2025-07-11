# typed: true
# enable-experimental-rbs-comments: true

# Sig parts hover

#: -> String
#^ hover: null
# ^ hover: (nothing)
#  ^ hover: (nothing)
#   ^ hover: (nothing)
#    ^ hover: (nothing)
def hover_def_sig1; T.unsafe(nil); end

#: () -> String
#  ^ hover: (nothing)
#   ^ hover: (nothing)
def hover_sig2; T.unsafe(nil); end

#: (Integer, String) -> String
#          ^ hover: T.class_of(Integer)
#                   ^ TODO: Should be pointing to the sig itself?
#           ^ hover: # T::Private::Methods::DeclBuilder#params:
def hover_sig3(x, y); T.unsafe(nil); end

# Return type hover

#: -> String
#     ^ hover: T.class_of(String)
#          ^ hover: T.class_of(String)
def hover_return_1; T.unsafe(nil); end

#: -> void
#     ^ hover: # T::Private::Methods::DeclBuilder#void:
#        ^ hover: # T::Private::Methods::DeclBuilder#void:
def hover_return_2; T.unsafe(nil); end

# Positional params hover

# TODO: These are all wrong, somehow it's linking to the arg name location, not the type location
#: (String, Integer, Symbol) -> void
#   ^ hover: T.class_of(String)
#        ^ hover: T.class_of(String)
#           ^ hover: T.class_of(Integer)
#                 ^ hover: T.class_of(Integer)
#                     ^ hover: T.class_of(Symbol)
#                         ^ hover: T.class_of(Symbol)
def hover_pos_params_1(x, y, z); T.unsafe(nil); end

# Named params hover

#: (String x, Integer y, Symbol z) -> void
#   ^ hover: T.class_of(String)
#        ^ hover: T.class_of(String)
#          ^ hover: String
#             ^ hover: T.class_of(Integer)
#                   ^ hover: T.class_of(Integer)
#                     ^ hover: Integer
#                        ^ hover: T.class_of(Symbol)
#                             ^ hover: T.class_of(Symbol)
#                               ^ hover: Symbol
def hover_named_params_1(x, y, z); T.unsafe(nil); end

# Keyword params hover

#: (x: String, y: Integer, z: Symbol) -> void
#   ^ hover: String
#      ^ hover: T.class_of(String)
#           ^ hover: T.class_of(String)
#              ^ hover: Integer
#                 ^ hover: T.class_of(Integer)
#                       ^ hover: T.class_of(Integer)
#                          ^ hover: Symbol
#                              ^ hover: T.class_of(Symbol)
#                                  ^ hover: T.class_of(Symbol)
def hover_keyword_params_1(x:, y:, z:); T.unsafe(nil); end

# Block params hover

#: () { -> String } -> void
#     ^ hover: # T.proc:
#          ^ hover: T.class_of(String)
#               ^ hover: T.class_of(String)
def hover_block_params_1(&block); T.unsafe(nil); end

# Attr sig hover

#: String
#^ hover: null
# ^ hover: (nothing)
#  ^ hover: T.class_of(String)
#       ^ hover: T.class_of(String)
attr_reader :hover_attr_sig1

# Annotations hover

# @final
# ^ hover: null
#  ^ hover: Symbol(:final)
#      ^ hover: Symbol(:final)
#: -> void
def hover_annot_1; end

# @final
# ^ hover: null
#  ^ hover: Symbol(:final)
#      ^ hover: Symbol(:final)
#: -> void
def hover_annot_1; end

# @overridable
# ^ hover: null
#  ^ hover: # T::Private::Methods::DeclBuilder#overridable:
#            ^ hover: # T::Private::Methods::DeclBuilder#overridable:
#: -> void
def hover_annot_2; end

# @final
# ^ hover: null
#  ^ hover: Symbol(:final)
#      ^ hover: Symbol(:final)
#: Integer
attr_reader :hover_attr_annot_1
