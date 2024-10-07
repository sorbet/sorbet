# typed: false

alias new_method1 new_method1 # Works with bare references

alias :new_method2 :new_method2 # Works with Symbols

# `alias` can't be used with instance and class variables
# alias @new_ivar @old_ivar
# alias @@new_cvar @@old_cvar
alias $new_global $old_global # Works with global variables
