# typed: true

local_variable1 = 123

local_variable2 = this_is_a_method_call
#                 ^^^^^^^^^^^^^^^^^^^^^ error: Method `this_is_a_method_call` does not exist on `T.class_of(<root>)`
