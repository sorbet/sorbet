#!/usr/bin/env bash
set -exuo pipefail

pr_id="$BUILDKITE_PULL_REQUEST"

pr_author=$(curl --silent --fail --netrc --location "https://api.github.com/repos/stripe/sorbet/pulls/${pr_id}" | jq '.["user"]["login"]'| tr -d '"')

potential_reviewers=$(comm -23 <(sort .buildkite/reviewer-list.txt) <(echo "$pr_author"))
chosen_reviewer=$(echo "$potential_reviewers"|shuf -n 1)
current_reviewer_count=$(curl --silent --fail --netrc --location "https://api.github.com/repos/stripe/sorbet/pulls/${pr_id}/requested_reviewers" | jq '.["users"] | length')

payload=$(
  jq --null-input \
     --arg reviewer "$chosen_reviewer" \
     '{reviewers: [$reviewer], team_reviewers: []}'
)


if [ "0" == "$current_reviewer_count" ]; then
    curl --fail \
         --netrc \
         --silent \
         --location \
         --data "$payload" \
         "https://api.github.com/repos/stripe/sorbet/pulls/${pr_id}/requested_reviewers"
fi
