#!/bin/bash

set -ev
curl -s https://static.rust-lang.org/rustup.sh | sh -s -- --channel=nightly
