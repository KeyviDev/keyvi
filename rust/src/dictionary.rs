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

pub struct Dictionary {
    dict: *mut root::keyvi_dictionary,
}

unsafe impl Send for Dictionary {}

unsafe impl Sync for Dictionary {}

impl Dictionary {
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

    pub fn statistics(&self) -> String {
        let c_buf: *mut ::std::os::raw::c_char =
            unsafe { root::keyvi_dictionary_get_statistics(self.dict) };
        KeyviString::new(c_buf).to_owned()
    }

    pub fn size(&self) -> u64 {
        unsafe { root::keyvi_dictionary_get_size(self.dict) }
    }

    pub fn get(&self, key: &str) -> KeyviMatch {
        let key_c = CString::new(key).unwrap();
        let match_ptr = unsafe { root::keyvi_dictionary_get(self.dict, key_c.as_ptr()) };
        KeyviMatch::new(match_ptr)
    }

    pub fn get_all_items(&self) -> KeyviMatchIterator {
        let ptr = unsafe { root::keyvi_dictionary_get_all_items(self.dict) };
        KeyviMatchIterator::new(ptr)
    }

    pub fn get_prefix_completions(&self, key: &str, cutoff: u64) -> KeyviMatchIterator {
        let key_c = CString::new(key).unwrap();
        let ptr = unsafe {
            root::keyvi_dictionary_get_prefix_completions(self.dict, key_c.as_ptr(), cutoff)
        };
        KeyviMatchIterator::new(ptr)
    }

    pub fn get_fuzzy(&self, key: &str, max_edit_distance: u64) -> KeyviMatchIterator {
        let key_c = CString::new(key).unwrap();
        let ptr = unsafe {
            root::keyvi_dictionary_get_fuzzy(self.dict, key_c.as_ptr(), max_edit_distance)
        };
        KeyviMatchIterator::new(ptr)
    }

    pub fn get_multi_word_completions(&self, key: &str, cutoff: u64) -> KeyviMatchIterator {
        let key_c = CString::new(key).unwrap();
        let ptr = unsafe {
            root::keyvi_dictionary_get_multi_word_completions(self.dict, key_c.as_ptr(), cutoff)
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
