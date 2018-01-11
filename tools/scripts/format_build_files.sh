#!/bin/bash

set -e

cd "$(dirname "$0")/../.."

SHA=ad4b2d42459f69158e81dcd1d6fc9666efad9d7b

if [ ! -x "$(which buildifier)" ]; then

  if [[ ! -x "$(which go)" ]]; then
    if [[ ! -x "$(which brew)" ]]; then
        echo -ne "\\033[0;31m"
        echo "You need to install golang."
        echo -ne "\\033[0m"
        exit 1
    fi
    echo "Installing a golang for you."
    brew install golang
  fi

  if [[ -z "$GOPATH" ]]; then
    export GOPATH=~/golang
    export PATH=$GOPATH/bin:$PATH
    mkdir -p $GOPATH
  fi

  if [ ! -x "$(which buildifier)" ]; then
    go get -d -u github.com/bazelbuild/buildifier/buildifier
    pushd $GOPATH/src/github.com/bazelbuild/buildifier/
    git reset --hard $SHA
    git clean -fdx --quiet
    go generate github.com/bazelbuild/buildifier/build
    go install github.com/bazelbuild/buildifier/buildifier
    popd
  fi
fi
if [ "$1" == "-t" ]; then
  OUTPUT=$(find . -name BUILD -not -path "./bazel-*" -print0 | xargs -0 buildifier -v -mode=check || :)
  if [ -n "$OUTPUT" ]; then
    echo -ne "\\e[1;31m"
    echo "☢️☢️  Some bazel files need to be reformatted! ☢️☢️"
    echo "$OUTPUT"
    echo -ne "\\e[0m"
    echo -e "✨✨ Run \\e[97;1;42m ./tools/scripts/format_build_files.sh\\e[0m to fix them up.  ✨✨"
    exit 1
  fi
else
  exec find . -name BUILD -not -path "./bazel-*" -print0 | xargs -0 buildifier -v -mode=fix
fi
