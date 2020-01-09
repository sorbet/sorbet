# typed: true

# We never want to show constants that have angle brackets in them to the user.
# Such names aren't human-writable, yet they exist in Sorbet's payload for
# various reasons.

Magic # error: Unable to resolve
#    ^ completion: <Magic>
