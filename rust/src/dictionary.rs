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
 *  dictionary.rs
 *
 *  Created on: September 4, 2017
 *  Author: Narek Gharibyan <narekgharibyan@gmail.com>
 *          Subu <subu@cliqz.com>
 */

use std::ffi::CString;
use std::io;

use bindings::*;
use keyvi_match::KeyviMatch;
use keyvi_match_iterator::KeyviMatchIterator;
use keyvi_string::KeyviString;

/// Read-only access to a compiled keyvi dictionary file.
///
/// A `Dictionary` is thread-safe and can be shared across threads.
///
/// # Example
///
/// ```no_run
/// use keyvi::dictionary::Dictionary;
///
/// let dict = Dictionary::new("data.kv").unwrap();
/// let m = dict.get("my_key");
/// println!("{}", m.get_value_as_string());
/// ```
pub struct Dictionary {
    dict: *mut root::keyvi_dictionary,
}

unsafe impl Send for Dictionary {}

unsafe impl Sync for Dictionary {}

impl Dictionary {
    /// Open a dictionary from the given file path.
    ///
    /// Returns an error if the file does not exist or cannot be loaded.
    pub fn new(filename: &str) -> io::Result<Dictionary> {
        let fn_c = CString::new(filename)?;
        let ptr = unsafe { root::keyvi_create_dictionary(fn_c.as_ptr()) };
        if ptr.is_null() {
            Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "could not load file",
            ))
        } else {
            Ok(Dictionary { dict: ptr })
        }
    }

    /// Open a dictionary with a specific loading strategy.
    ///
    /// Available strategies (from [`LoadingStrategy`](crate::LoadingStrategy)):
    /// - `populate` — eagerly load the full dictionary into memory.
    /// - `lazy` — load pages on demand via memory-mapped I/O.
    /// - `populate_lazy` — advise the OS to load pages but don't block.
    pub fn with_loading_strategy(
        filename: &str,
        loading_strategy: root::keyvi::dictionary::LoadingStrategy,
    ) -> io::Result<Dictionary> {
        let fn_c = CString::new(filename)?;
        let ptr = unsafe {
            root::keyvi_create_dictionary_with_loading_strategy(fn_c.as_ptr(), loading_strategy)
        };
        if ptr.is_null() {
            Err(io::Error::new(
                io::ErrorKind::InvalidInput,
                "could not load file",
            ))
        } else {
            Ok(Dictionary { dict: ptr })
        }
    }

    /// Return dictionary statistics as a JSON string.
    pub fn statistics(&self) -> String {
        let c_buf: *mut ::std::os::raw::c_char =
            unsafe { root::keyvi_dictionary_get_statistics(self.dict) };
        KeyviString::new(c_buf).to_owned()
    }

    /// Return the number of entries in the dictionary.
    pub fn size(&self) -> usize {
        unsafe { root::keyvi_dictionary_get_size(self.dict) }
    }

    /// Look up a key. Returns a [`KeyviMatch`] (call [`KeyviMatch::is_empty`] to
    /// check whether the key was found).
    pub fn get(&self, key: &str) -> KeyviMatch {
        let match_ptr = unsafe {
            root::keyvi_dictionary_get(
                self.dict,
                key.as_ptr() as *const ::std::os::raw::c_char,
                key.len() as usize,
            )
        };
        KeyviMatch::new(match_ptr)
    }

    /// Return an iterator over all key-value pairs in the dictionary.
    pub fn get_all_items(&self) -> KeyviMatchIterator {
        let ptr = unsafe { root::keyvi_dictionary_get_all_items(self.dict) };
        KeyviMatchIterator::new(ptr)
    }

    /// Return completions for the given prefix, up to `cutoff` results.
    pub fn get_prefix_completions(&self, key: &str, cutoff: usize) -> KeyviMatchIterator {
        let ptr = unsafe {
            root::keyvi_dictionary_get_prefix_completions(
                self.dict,
                key.as_ptr() as *const ::std::os::raw::c_char,
                key.len() as usize,
                cutoff,
            )
        };
        KeyviMatchIterator::new(ptr)
    }

    /// Return fuzzy matches within the given edit distance.
    pub fn get_fuzzy(&self, key: &str, max_edit_distance: usize) -> KeyviMatchIterator {
        let ptr = unsafe {
            root::keyvi_dictionary_get_fuzzy(
                self.dict,
                key.as_ptr() as *const ::std::os::raw::c_char,
                key.len() as usize,
                max_edit_distance,
            )
        };
        KeyviMatchIterator::new(ptr)
    }

    /// Return multi-word completions for the given prefix, up to `cutoff` results.
    pub fn get_multi_word_completions(&self, key: &str, cutoff: usize) -> KeyviMatchIterator {
        let ptr = unsafe {
            root::keyvi_dictionary_get_multi_word_completions(
                self.dict,
                key.as_ptr() as *const ::std::os::raw::c_char,
                key.len() as usize,
                cutoff,
            )
        };
        KeyviMatchIterator::new(ptr)
    }
}

impl Drop for Dictionary {
    fn drop(&mut self) {
        unsafe {
            root::keyvi_dictionary_destroy(self.dict);
        }
    }
}
