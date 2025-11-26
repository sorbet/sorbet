# typed: false

T.any(Integer, String x)
#                     ^ error: unexpected token tIDENTIFIER
T.any(Integer, String Float)
#                     ^^^^^ error: unexpected token tCONSTANT
T.any(Integer, String @x)
#                     ^^ error: unexpected token tIVAR
T.any(Integer, String puts(x))
#                     ^^^^ error: unexpected token tIDENTIFIER
#                            ^ error: unexpected token ")"
T.any(Integer, String T.nilable(Float))
#                     ^ error: unexpected token tCONSTANT
#                                     ^ error: unexpected token ")"

def even_missing_paren
  T.any(Integer, String x
        #               ^ error: unexpected token tIDENTIFIER
end

