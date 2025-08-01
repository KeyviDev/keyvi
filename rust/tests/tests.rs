extern crate flate2;
extern crate rand;
extern crate rayon;
extern crate serde_json;
extern crate snap;
extern crate zstd;

extern crate keyvi;

#[cfg(test)]
mod tests {
    use std::io::Read;

    use flate2::read::ZlibDecoder;
    use rand;
    use rand::Rng;
    use rayon::prelude::*;
    use serde_json::{value, Value};
    use snap::raw::Decoder;

    use keyvi::dictionary;

    #[test]
    fn dictionary_error() {
        let dict = dictionary::Dictionary::new("test_data/fake_file_name.kv");
        assert!(dict.is_err());
        assert_eq!(
            dict.err().unwrap().to_string().as_str(),
            "could not load file"
        );
    }

    #[test]
    fn dictionary_size() {
        let dict = dictionary::Dictionary::new("test_data/test.kv").unwrap();
        assert_eq!(dict.size(), 5);
    }

    #[test]
    fn match_string() {
        let m = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get("a");
        assert_eq!(m.matched_string(), "a");
    }

    #[test]
    fn match_value_int() {
        let m = dictionary::Dictionary::new("test_data/completion_test.kv")
            .unwrap()
            .get("mozilla footprint");
        match m.get_value() {
            Value::Number(n) => assert_eq!(n.as_i64().unwrap(), 30),
            _ => assert!(false),
        }
    }

    #[test]
    fn match_msgpacked_value_int() {
        let m = dictionary::Dictionary::new("test_data/completion_test.kv")
            .unwrap()
            .get("mozilla footprint");
        assert_eq!(m.get_msgpacked_value(), vec![30]);
    }

    #[test]
    fn match_value_array() {
        let m = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get("a");
        match m.get_value() {
            Value::Array(n) => assert_eq!(n, vec![12, 13]),
            _ => assert!(false),
        }
    }

    #[test]
    fn match_msgpacked_value_array() {
        let m = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get("a");
        assert_eq!(m.get_msgpacked_value(), vec![146, 12, 13]);
    }

    #[test]
    fn match_msgpacked_value_non_existing_key() {
        let d = dictionary::Dictionary::new("test_data/test.kv").unwrap();
        let m = d.get("non-existing-key");
        assert!(m.get_value_as_string().is_empty());

        let m = d.get("non-existing-key-with-\0-in-middle");
        assert!(m.get_value_as_string().is_empty());
    }

    #[test]
    fn match_msgpacked_value_compressed_array() {
        let m = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get("a");

        assert_eq!(
            m.get_msgpacked_value_compressed(keyvi::Compression::NO_COMPRESSION),
            vec![146, 12, 13]
        );

        let mut snap_decoder = Decoder::new();
        let value_compressed_snap =
            m.get_msgpacked_value_compressed(keyvi::Compression::SNAPPY_COMPRESSION);
        let value_uncompressed_snap = snap_decoder.decompress_vec(&value_compressed_snap);
        assert_eq!(value_uncompressed_snap.unwrap(), vec![146, 12, 13]);

        let value_compressed_zstd =
            m.get_msgpacked_value_compressed(keyvi::Compression::ZSTD_COMPRESSION);
        let value_uncompressed_zstd: Vec<u8> =
            zstd::decode_all(value_compressed_zstd.as_slice()).unwrap();
        assert_eq!(value_uncompressed_zstd, vec![146, 12, 13]);

        let value_compressed_zlib =
            m.get_msgpacked_value_compressed(keyvi::Compression::ZLIB_COMPRESSION);
        let mut zlib_decoder = ZlibDecoder::new(value_compressed_zlib.as_slice());
        let mut value_uncompressed_zlib: Vec<u8> = Vec::new();
        let _ = zlib_decoder.read_to_end(&mut value_uncompressed_zlib);
        assert_eq!(value_uncompressed_zlib, vec![146, 12, 13]);
    }

    #[test]
    fn match_value() {
        let d = dictionary::Dictionary::new("test_data/test.kv").unwrap();
        let m = d.get("a");
        assert_eq!(m.get_value_as_string(), "[12,13]");

        let m = d.get("d\0");
        assert_eq!(m.get_value_as_string(), "[1,2]");

        let m = d.get("e\0f");
        assert_eq!(m.get_value_as_string(), "[3,4]");
    }

    #[test]
    fn match_is_empty() {
        let m = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get("a");
        assert_eq!(m.is_empty(), false);
    }

    #[test]
    fn match_iterator_count() {
        let mit = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get_prefix_completions("a", 10);
        assert_eq!(mit.count(), 1);
    }

    #[test]
    fn match_iterator_values() {
        let mit = dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get_prefix_completions("a", 10);
        for m in mit {
            assert_eq!(m.matched_string(), "a");
            assert_eq!(m.get_value_as_string(), "[12,13]");
        }
    }

    #[test]
    fn match_iterator_into() {
        for m in dictionary::Dictionary::new("test_data/test.kv")
            .unwrap()
            .get_prefix_completions("a", 10)
        {
            let (k, v) = m.into();
            assert_eq!(k, "a");

            match v {
                Value::Array(n) => assert_eq!(n, vec![12, 13]),
                _ => assert!(false),
            }
        }
    }

    #[test]
    fn get_all_items() {
        let expected_items = [("a", "[12,13]"), ("b", "[12,13]"), ("c", "[14,15]")];
        let dict = dictionary::Dictionary::with_loading_strategy(
            "test_data/test.kv",
            keyvi::LoadingStrategy::populate,
        )
        .unwrap();

        for (item, expected_item) in dict.get_all_items().zip(&expected_items) {
            assert_eq!(item.matched_string(), expected_item.0);
            assert_eq!(item.get_value_as_string(), expected_item.1);
        }
    }

    #[test]
    fn multi_word_completions() {
        let mut values = vec![
            ("80", "mozilla firefox"),
            ("43", "mozilla fans"),
            ("30", "mozilla footprint"),
            ("12", "mozilla firebird"),
        ];
        values.sort();
        let new_values: Vec<(String, String)> = values
            .into_iter()
            .map(|(x, y)| (x.into(), y.into()))
            .collect();

        let mit = dictionary::Dictionary::new("test_data/completion_test.kv")
            .unwrap()
            .get_multi_word_completions("mozilla f", 10);
        let mut a: Vec<(String, String)> = mit
            .map(|m| (m.get_value_as_string(), m.matched_string()))
            .collect();
        a.sort();

        assert_eq!(new_values, a);
    }

    #[test]
    fn prefix_completions() {
        let d = dictionary::Dictionary::new("test_data/completion_test.kv").unwrap();

        let mut all_prefix_completions: Vec<String> = d
            .get_prefix_completions("m", 10000)
            .map(|m| (m.matched_string()))
            .collect();
        all_prefix_completions.sort();
        assert_eq!(
            all_prefix_completions,
            vec![
                "mozilla fans",
                "mozilla firebird",
                "mozilla firefox",
                "mozilla footprint"
            ]
        );

        let mut some_prefix_completions: Vec<String> = d
            .get_prefix_completions("m", 2)
            .map(|m| (m.matched_string()))
            .collect();
        some_prefix_completions.sort();
        assert_eq!(
            some_prefix_completions,
            vec!["mozilla fans", "mozilla firefox"]
        );
    }

    #[test]
    fn multi_word_completions_cutoff() {
        let mut values = vec![("80", "mozilla firefox")];
        values.sort();
        let new_values: Vec<(String, String)> = values
            .into_iter()
            .map(|(x, y)| (x.into(), y.into()))
            .collect();

        let mit = dictionary::Dictionary::new("test_data/completion_test.kv")
            .unwrap()
            .get_multi_word_completions("mozilla f", 1);
        let mut a: Vec<(String, String)> = mit
            .map(|m| (m.get_value_as_string(), m.matched_string()))
            .collect();
        a.sort();

        assert_eq!(new_values, a);
    }

    #[test]
    fn fuzzy_completions() {
        let mut values = vec![("22", "aabc"), ("55", "aabcül")];
        values.sort();
        let new_values: Vec<(String, String)> = values
            .into_iter()
            .map(|(x, y)| (x.into(), y.into()))
            .collect();

        let mit = dictionary::Dictionary::new("test_data/fuzzy.kv")
            .unwrap()
            .get_fuzzy("aafcül", 3);
        let mut a: Vec<(String, String)> = mit
            .map(|m| (m.get_value_as_string(), m.matched_string()))
            .collect();
        a.sort();

        assert_eq!(new_values, a);
    }

    #[test]
    fn fuzzy_completions_non_ascii() {
        let mut values = vec![
            ("10188", "tüv in"),
            ("331", "tüv i"),
            ("45901", "tüv süd"),
            ("46052", "tüv nord"),
        ];
        values.sort();
        let new_values: Vec<(String, String)> = values
            .into_iter()
            .map(|(x, y)| (x.into(), y.into()))
            .collect();

        let mit = dictionary::Dictionary::new("test_data/fuzzy_non_ascii.kv")
            .unwrap()
            .get_fuzzy("tüc", 6);
        let mut a: Vec<(String, String)> = mit
            .map(|m| (m.get_value_as_string(), m.matched_string()))
            .collect();
        a.sort();

        assert_eq!(new_values, a);
    }

    #[test]
    fn fuzzy_completions_with_score() {
        let mut values = vec![("22", "aabc", "3.0"), ("55", "aabcül", "1.0")];
        values.sort();
        let new_values: Vec<(String, String, String)> = values
            .into_iter()
            .map(|(x, y, z)| (x.into(), y.into(), z.into()))
            .collect();

        let mit = dictionary::Dictionary::new("test_data/fuzzy.kv")
            .unwrap()
            .get_fuzzy("aafcül", 3);
        let mut a: Vec<(String, String, String)> = mit
            .map(|m| {
                (
                    m.get_value_as_string(),
                    m.matched_string(),
                    format!("{:.*}", 1, m.get_score()),
                )
            })
            .collect();
        a.sort();
        assert_eq!(new_values, a);
    }

    #[test]
    fn dictionary_parallel_test() {
        let mut rng = rand::thread_rng();
        let mut keys = Vec::new();
        for _ in 0..10000 {
            let letter: char = rng.gen_range(b'a'..b'c') as char;
            keys.push(letter.to_string())
        }

        let dictionary = dictionary::Dictionary::new("test_data/test.kv").unwrap();

        let sequential_values: Vec<Value> = keys
            .iter()
            .map(|key| dictionary.get(key).get_value())
            .collect();
        let parallel_values: Vec<Value> = keys
            .par_iter()
            .map(|key| dictionary.get(key).get_value())
            .collect();

        assert_eq!(sequential_values, parallel_values);
    }
}
