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
 *  keyvi_string.rs
 *
 *  Created on: September 4, 2017
 *  Author: Narek Gharibyan <narekgharibyan@gmail.com>
 *          Subu <subu@cliqz.com>
 */

use std::ffi::CStr;
use std::ops::Deref;

use bindings::*;

pub struct KeyviString {
    str_: *mut ::std::os::raw::c_char,
}

impl KeyviString {
    pub fn new(str: *mut ::std::os::raw::c_char) -> KeyviString {
        KeyviString { str_: str }
    }
}

impl Drop for KeyviString {
    fn drop(&mut self) {
        unsafe {
            root::keyvi_string_destroy(self.str_);
        }
    }
}

impl Deref for KeyviString {
    type Target = str;

    fn deref<'a>(&'a self) -> &'a str {
        let c_str = unsafe { CStr::from_ptr(self.str_) };
        c_str.to_str().unwrap()
    }
}
