# typed: strict

class ::Owner::Qualified; end # error: File belongs to package `Prelude` but defines a constant that does not match this namespace

module ::Owner::Deep::Patch; end # error: File belongs to package `Prelude` but defines a constant that does not match this namespace
