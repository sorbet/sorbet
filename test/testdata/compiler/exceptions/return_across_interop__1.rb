# frozen_string_literal: true
# typed: true
# compiled: true

def compiled_ensure(&blk)
  begin
    p "compiled_ensure begin"
    yield
  ensure
    p "compiled_ensure ensure"
  end
end

def compiled_rescue(&blk)
  begin
    p "compiled_rescue begin"
    yield
  rescue TypeError
    p "compiled_rescue rescue"
  ensure
    p "compiled_rescue ensure"
  end
end

require_relative 'return_across_interop__2'
  
