# typed: false

def comma_after_pos_args(x, y,) # parser-error: unexpected token ","
end

def comma_after_kwargs(x:, y:,) # parser-error: unexpected token ","
end

def comma_after_all_kwargs(a, b, x:, y:,) # parser-error: unexpected token ","
end
