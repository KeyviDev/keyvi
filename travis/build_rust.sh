#!/usr/bin/env bash
set -ev

cd rust
cargo build --verbose
cargo test --verbose
