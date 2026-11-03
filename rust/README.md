# keyvi - Rust bindings

Rust bindings for [keyvi](https://github.com/KeyviDev/keyvi) — a key value index based on finite state machines, optimized for size and lookup speed.

## Usage

Add keyvi to your `Cargo.toml`:

```toml
[dependencies]
keyvi = "0.7"
```

### Example

```rust
use keyvi::dictionary::Dictionary;

let dict = Dictionary::new("data.kv").unwrap();
let m = dict.get("my_key");

if !m.is_empty() {
    println!("{}: {}", m.matched_string(), m.get_value_as_string());
}

// iterate over prefix completions
for m in dict.get_prefix_completions("moz", 10) {
    println!("{}: {}", m.matched_string(), m.get_value_as_string());
}

// fuzzy matching
for m in dict.get_fuzzy("approximate", 2) {
    println!("{} (distance: {})", m.matched_string(), m.get_score());
}
```

## Documentation

Full API documentation is available at [keyvidev.github.io/keyvi](https://keyvidev.github.io/keyvi/rust/dictionary.html).

## Build from source

The crate builds the keyvi C++ core via CMake and generates Rust bindings with [bindgen](https://github.com/rust-lang/rust-bindgen). You need:

- A C++17 compiler (GCC >= 7 or Clang >= 5)
- CMake >= 3.13
- Boost >= 1.70

Then:

```sh
cargo build
cargo test
```
