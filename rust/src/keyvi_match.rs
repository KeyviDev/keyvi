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
 *  keyvi_match.rs
 *
 *  Created on: September 4, 2017
 *  Author: Narek Gharibyan <narekgharibyan@gmail.com>
 *          Subu <subu@cliqz.com>
 */

use std::slice;

use serde_json;

use bindings::*;
use keyvi_string::KeyviString;

/// A single match result from a dictionary lookup.
///
/// Implements `Into<(String, serde_json::Value)>` and
/// `Into<(String, serde_json::Value, f64)>` for convenient destructuring.
///
/// # Example
///
/// ```no_run
/// use keyvi::dictionary::Dictionary;
///
/// let dict = Dictionary::new("data.kv").unwrap();
/// let m = dict.get("my_key");
/// if !m.is_empty() {
///     println!("{}: {}", m.matched_string(), m.get_value_as_string());
/// }
/// ```
pub struct KeyviMatch {
    match_ptr_: *mut root::keyvi_match,
}

impl KeyviMatch {
    /// Create a new match from a raw pointer (internal use).
    pub fn new(match_ptr: *mut root::keyvi_match) -> KeyviMatch {
        KeyviMatch {
            match_ptr_: match_ptr,
        }
    }

    /// Return the value as a `serde_json::Value`.
    pub fn get_value(&self) -> serde_json::Value {
        let value_str = self.get_value_as_string();
        let json = match serde_json::from_str(value_str.as_str()) {
            Ok(j) => j,
            Err(e) => {
                eprintln!("Err: {}", e);
                serde_json::json!(null)
            }
        };
        json
    }

    /// Return the value as a JSON string.
    pub fn get_value_as_string(&self) -> String {
        let c_buf = unsafe { root::keyvi_match_get_value_as_string(self.match_ptr_) };
        KeyviString::new(c_buf).to_owned()
    }

    /// Return the raw value as a MessagePack-encoded byte vector.
    pub fn get_msgpacked_value(&self) -> Vec<u8> {
        let kv_bytes = unsafe { root::keyvi_match_get_msgpacked_value(self.match_ptr_) };
        let msgpacked_value = if kv_bytes.data_size == 0 {
            Vec::new()
        } else {
            unsafe {
                slice::from_raw_parts(kv_bytes.data_ptr, kv_bytes.data_size as usize).to_vec()
            }
        };
        unsafe { root::keyvi_bytes_destroy(kv_bytes) };

        msgpacked_value
    }

    /// Return the raw value as a compressed MessagePack-encoded byte vector.
    ///
    /// Available compression algorithms (from [`Compression`](crate::Compression)):
    /// `NO_COMPRESSION`, `ZLIB_COMPRESSION`, `SNAPPY_COMPRESSION`, `ZSTD_COMPRESSION`.
    pub fn get_msgpacked_value_compressed(
        &self,
        compression_algorithm: root::keyvi::compression::CompressionAlgorithm,
    ) -> Vec<u8> {
        let kv_bytes = unsafe {
            root::keyvi_match_get_msgpacked_value_compressed(self.match_ptr_, compression_algorithm)
        };
        let msgpacked_value = if kv_bytes.data_size == 0 {
            Vec::new()
        } else {
            unsafe {
                slice::from_raw_parts(kv_bytes.data_ptr, kv_bytes.data_size as usize).to_vec()
            }
        };
        unsafe { root::keyvi_bytes_destroy(kv_bytes) };

        msgpacked_value
    }

    /// Return whether this match is empty (key was not found).
    pub fn is_empty(&self) -> bool {
        unsafe { root::keyvi_match_is_empty(self.match_ptr_) }
    }

    /// Return the match score (e.g. edit distance for fuzzy matches, weight for completions).
    pub fn get_score(&self) -> f64 {
        unsafe { root::keyvi_match_get_score(self.match_ptr_) }
    }

    /// Return the matched key.
    pub fn matched_string(&self) -> String {
        let c_buf = unsafe { root::keyvi_match_get_matched_string(self.match_ptr_) };
        KeyviString::new(c_buf).to_owned()
    }
}

impl Into<(String, serde_json::Value)> for KeyviMatch {
    fn into(self) -> (String, serde_json::Value) {
        (self.matched_string(), self.get_value())
    }
}

impl Into<(String, serde_json::Value, f64)> for KeyviMatch {
    fn into(self) -> (String, serde_json::Value, f64) {
        (self.matched_string(), self.get_value(), self.get_score())
    }
}

impl Drop for KeyviMatch {
    fn drop(&mut self) {
        unsafe {
            root::keyvi_match_destroy(self.match_ptr_);
        }
    }
}
