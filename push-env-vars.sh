#!/bin/bash

# push env vars from overrides file to af

while read line ; do 
  af env-add scholar-issue-sync $line
done < env_overrides.inc
