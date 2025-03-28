# typed: true

case 0; in ^a; true; end # parser-error: no such local variable: a
