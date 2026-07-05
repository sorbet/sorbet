#!/bin/bash
main/sorbet --silence-dev-message -e 'class Foo; end' --stdout-hup-hack 2>&1
