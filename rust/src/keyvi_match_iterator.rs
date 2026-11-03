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
 *  keyvi_match_iterator.rs
 *
 *  Created on: September 4, 2017
 *  Author: Narek Gharibyan <narekgharibyan@gmail.com>
 *          Subu <subu@cliqz.com>
 */

use bindings::*;
use keyvi_match::KeyviMatch;

/// An iterator over match results from a dictionary lookup.
///
/// Implements the standard Rust [`Iterator`] trait with `Item = KeyviMatch`.
///
/// # Example
///
/// ```no_run
/// use keyvi::dictionary::Dictionary;
///
/// let dict = Dictionary::new("data.kv").unwrap();
/// for m in dict.get_prefix_completions("moz", 10) {
///     println!("{}: {}", m.matched_string(), m.get_value_as_string());
/// }
/// ```
pub struct KeyviMatchIterator {
    ptr_: *mut root::keyvi_match_iterator,
}

impl KeyviMatchIterator {
    /// Create a new match iterator from a raw pointer (internal use).
    pub fn new(ptr: *mut root::keyvi_match_iterator) -> KeyviMatchIterator {
        KeyviMatchIterator { ptr_: ptr }
    }
}

impl Iterator for KeyviMatchIterator {
    type Item = KeyviMatch;

    fn next(&mut self) -> Option<KeyviMatch> {
        let empty = unsafe { root::keyvi_match_iterator_empty(self.ptr_) };
        if empty {
            return None;
        } else {
            let ret = Some(KeyviMatch::new(unsafe {
                root::keyvi_match_iterator_dereference(self.ptr_)
            }));
            unsafe { root::keyvi_match_iterator_increment(self.ptr_) };
            return ret;
        }
    }
}

impl Drop for KeyviMatchIterator {
    fn drop(&mut self) {
        unsafe {
            root::keyvi_match_iterator_destroy(self.ptr_);
        }
    }
}
