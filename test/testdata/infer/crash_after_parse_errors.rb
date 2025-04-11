# typed: true
->e # parser-error: unexpected token tNL
r=e # error: Method `e` does not exist
r=e # error: Method `e` does not exist
r&&o # error: Method `o` does not exist
