#!/usr/bin/env bash
export GHC_EVENTLOG_SOCKET=/tmp/eventlog.sock # This only applies if using `startWait Nothing` in the Spec.hs

# tests will block until socket is read
cabal test --test-show-details=direct --test-option=--format=progress &
job_id=$! # save the process ID for the cabal test command
# this needs to wait until the Spec.hs module creates the socket before starting netcat
sleep 5 && nc -U /tmp/eventlog.sock > demo.eventlog
echo "we're done!"
wait $job_id # this waits for the cabal test command to finish before exiting the script to prevent race conditions
