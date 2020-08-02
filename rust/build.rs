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
 *  build.rs
 *
 *  Created on: September 4, 2017
 *  Author: Narek Gharibyan <narekgharibyan@gmail.com>
 *          Subu <subu@cliqz.com>
 */

extern crate bindgen;
extern crate cmake;

use std::env;
use std::path::PathBuf;

use cmake::Config;

fn main() {
    let dst = Config::new("keyvi_core/").build_target("keyvi_c").build();

    // Tell cargo to tell rustc to link keyvi
    println!("cargo:rustc-link-lib=dylib=keyvi_c");
    println!(
        "cargo:rustc-link-search=native={}",
        dst.join("build").display()
    );

    println!("Starting to generate bindings..");

    let bindings = bindgen::Builder::default()
        // The input header we would like to generate bindings for.
        .header("keyvi_core/keyvi/include/keyvi/c_api/c_api.h")
        .clang_arg("-x")
        .clang_arg("c++")
        .enable_cxx_namespaces()
        .layout_tests(true)
        .whitelist_function("keyvi_bytes_destroy")
        .whitelist_function("keyvi_string_destroy")
        .whitelist_function("keyvi_create_dictionary")
        .whitelist_function("keyvi_dictionary_destroy")
        .whitelist_function("keyvi_dictionary_get")
        .whitelist_function("keyvi_dictionary_get_all_items")
        .whitelist_function("keyvi_dictionary_get_fuzzy")
        .whitelist_function("keyvi_dictionary_get_multi_word_completions")
        .whitelist_function("keyvi_dictionary_get_prefix_completions")
        .whitelist_function("keyvi_dictionary_get_size")
        .whitelist_function("keyvi_dictionary_get_statistics")
        .whitelist_function("keyvi_match_destroy")
        .whitelist_function("keyvi_match_get_matched_string")
        .whitelist_function("keyvi_match_get_msgpacked_value")
        .whitelist_function("keyvi_match_get_score")
        .whitelist_function("keyvi_match_get_value_as_string")
        .whitelist_function("keyvi_match_is_empty")
        .whitelist_function("keyvi_match_iterator_dereference")
        .whitelist_function("keyvi_match_iterator_destroy")
        .whitelist_function("keyvi_match_iterator_empty")
        .whitelist_function("keyvi_match_iterator_increment")
        // Finish the builder and generate the bindings.
        .generate()
        // Unwrap the Result and panic on failure.
        .expect("Unable to generate bindings");

    println!("Saving to bindings..");
    // Write the bindings to the $OUT_DIR/bindings.rs file.
    let out_path = PathBuf::from(env::var("OUT_DIR").unwrap());
    bindings
        .write_to_file(out_path.join("bindings.rs"))
        .expect("Couldn't write bindings!");
}
