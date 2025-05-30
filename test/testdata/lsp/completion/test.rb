MyAlias = T.type_alias { Integer }
# ^      completion: ARGF, ...
#      ^ completion: MyAlias

# We keep the typed sigil down here to ensure that it's possible to accidentally
# calculate a negative offset into the file.
# typed: true
