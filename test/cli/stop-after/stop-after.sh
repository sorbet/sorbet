#!/bin/bash

# Ensure that every suggestion printed by --stop-after x is a valid suggestion.
for option in $(main/sorbet --stop-after x -e '1' 2>&1 | ag 'Valid values' | cut -d ':' -f 2- | sed -e 's/,//g'); do
  # Hide the No errors so that adding new stop-after doesn't require exp update
  if ! main/sorbet --stop-after "$option" -e 1 2>&1 | grep -v 'No errors'; then
    exit 1
  fi
done
