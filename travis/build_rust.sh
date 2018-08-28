#!/usr/bin/env bash
set -ex

cd /io/rust

cargo build --verbose
cargo test --verbose
