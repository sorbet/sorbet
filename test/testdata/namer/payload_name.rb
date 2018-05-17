# typed: strict

# A facinating bug. Since `raise` in a method on Kernel, it is in the payload,
# so it's Name will be entered into the table during payload. If the same name
# is used from userland it will re-use the same Name but that name won't be
# owned by the userland GlobalState. This hit an enforce before we tracked the
# parent GlobalState.

1 + "raise" # error: Expression passed as an argument
