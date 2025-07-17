# typed: false

local_variable1 = 123

local_variable2 = this_is_a_method_call

local_variable2 = local_variable1 # should parse as local variable lookup
