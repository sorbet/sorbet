# typed: false

def comma_after_pos_args(x, y,) # error: unexpected token ","
end

def comma_after_kwargs(x:, y:,) # error: unexpected token ","
end

def comma_after_all_kwargs(a, b, x:, y:,) # error: unexpected token ","
end
