#!/bin/bash
set -e

# push the pay-server tag
git push origin origin/master:pay-server

# send the binary to dev servers
henson deploy --qa ruby-typer -r origin/pay-server --quit-after-success
