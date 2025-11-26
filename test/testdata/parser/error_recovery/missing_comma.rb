# typed: false

T.any(Integer, String x)
#                    ^ error: missing token ","
T.any(Integer, String Float)
#                    ^ error: missing token ","
T.any(Integer, String @x)
#                    ^ error: missing token ","
T.any(Integer, String puts(x))
#                    ^ error: missing token ","
T.any(Integer, String T.nilable(Float))
#                    ^ error: missing token ","

def even_missing_paren
  T.any(Integer, String x
#                      ^ error: missing token ","
end # error: unterminated (

