# typed: true

def origin(x = nil); end
#   ^ apply-rename: [A] newName: ORIGIN placeholderText: origin

origin()

# This will technically turn into an error about "could not resolve constant"
# but it seems easy enough for the user to look at those errors and add the
# required parens on their own.
origin

# Crazily enough it doesn't even require parens to parse as a method name, it
# just needs parens or any argument.
origin 'foo'
