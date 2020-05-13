#!/usr/bin/env bash

find src -name 'test_*.js' | xargs -I {} node ./scripts/test.js ../{}
