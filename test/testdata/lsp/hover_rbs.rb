# typed: true
# enable-experimental-rbs-signatures: true

# Sig parts hover

#: -> String
#^ hover: # T::Sig::WithoutRuntime.sig:
# ^ hover: # T::Sig::WithoutRuntime.sig:
#  ^ hover: # T::Sig::WithoutRuntime.sig:
#   ^ hover: # T::Sig::WithoutRuntime.sig:
#    ^ hover: # T::Sig::WithoutRuntime.sig:
def hover_sig1; T.unsafe(nil); end

#: () -> String
#  ^ hover: # T::Sig::WithoutRuntime.sig:
#   ^ hover: # T::Sig::WithoutRuntime.sig:
def hover_sig2; T.unsafe(nil); end

#: (Integer, String) -> String
#          ^ hover: Integer
#                   ^ TODO: Should be pointing to the sig itself?
#           ^ hover: # T::Sig::WithoutRuntime.sig:
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
#   ^ hover: String
#        ^ hover: String
#           ^ hover: Integer
#                 ^ hover: Integer
#                     ^ hover: Symbol
#                         ^ hover: Symbol
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
def hover_keyword_params_1(x, y, z); T.unsafe(nil); end

# Block params hover

#: () { -> String } -> void
#     ^ hover: # T::Sig::WithoutRuntime.sig:
#              ^ TODO: RBS blocks do not have location yet
#          ^ hover: T.class_of(String)
#               ^ hover: T.class_of(String)
def hover_block_params_1(&block); T.unsafe(nil); end
