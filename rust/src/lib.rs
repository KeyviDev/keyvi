/* * keyvi - A key value store.
 *
 * Copyright 2017   Narek Gharibyan <narekgharibyan@gmail.com>
 *                  Subu <subu@cliqz.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * lib.rs
 *
 *  Created on: September 4, 2017
 *  Author: Narek Gharibyan <narekgharibyan@gmail.com>
 *          Subu <subu@cliqz.com>
 */

//! Rust bindings for keyvi — a key value index based on finite state machines.
//!
//! Keyvi is optimized for size and lookup speed. The underlying FST data structure
//! is very space efficient and supports various forms of approximate matching
//! including fuzzy string matching.
//!
//! # Example
//!
//! ```no_run
//! use keyvi::dictionary::Dictionary;
//!
//! let dict = Dictionary::new("data.kv").unwrap();
//! let m = dict.get("my_key");
//! if !m.is_empty() {
//!     println!("{}: {}", m.matched_string(), m.get_value_as_string());
//! }
//! ```

#![allow(non_upper_case_globals)]
#![allow(non_camel_case_types)]
#![allow(non_snake_case)]
#![crate_type = "lib"]

extern crate serde_json;

mod bindings;
pub mod dictionary;
pub mod keyvi_match;
pub mod keyvi_match_iterator;
pub mod keyvi_string;

/// Compression algorithms for msgpacked value retrieval.
pub type Compression = bindings::root::keyvi::compression::CompressionAlgorithm;
/// Loading strategies controlling how dictionary data is mapped into memory.
pub type LoadingStrategy = bindings::root::keyvi::dictionary::loading_strategy_types;
