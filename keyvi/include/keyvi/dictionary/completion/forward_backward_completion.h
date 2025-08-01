/* * keyvi - A key value store.
 *
 * Copyright 2015 Hendrik Muhs<hendrik.muhs@gmail.com>
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
 * forward_backward_completion.h
 *
 *  Created on: Sep 2, 2014
 *      Author: hendrik
 */

#ifndef KEYVI_DICTIONARY_COMPLETION_FORWARD_BACKWARD_COMPLETION_H_
#define KEYVI_DICTIONARY_COMPLETION_FORWARD_BACKWARD_COMPLETION_H_

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "keyvi/dictionary/completion/prefix_completion.h"
#include "keyvi/dictionary/util/bounded_priority_queue.h"

// #define ENABLE_TRACING
#include "keyvi/dictionary/util/trace.h"

namespace keyvi {
namespace dictionary {
namespace completion {

/**
 * Port of forward->backward suggester code from python to C++
 */
class ForwardBackwardCompletion final {
 public:
  ForwardBackwardCompletion(dictionary_t forward_dictionary, dictionary_t backward_dictionary)
      : forward_completions_(forward_dictionary), backward_completions_(backward_dictionary) {}

  struct result_compare {
    bool operator()(const match_t& m1, const match_t& m2) const { return m1->GetScore() < m2->GetScore(); }
  };

  MatchIterator::MatchIteratorPair GetCompletions(const std::string& query, int number_of_results = 10) {
    // get query length
    const size_t query_length = query.size();

    // get tokens
    std::vector<std::string> strs;
    boost::split(strs, query, boost::is_any_of("\t "));
    size_t number_of_tokens = strs.size();

    TRACE("Query: %s length %d tokens %d", query, query_length, number_of_tokens);

    // priority queue for pruning results
    util::BoundedPriorityQueue<uint32_t> best_scores(2 * number_of_results);
    std::vector<match_t> results;

    for (auto& match : forward_completions_.GetCompletions(query, number_of_results)) {
      uint32_t weight = match->GetWeight();

      // put the weight into the priority queue
      best_scores.Put(weight);
      match->SetScore(weight);
      results.push_back(match);

      TRACE("Forward Completion: %s %d", match.GetMatchedString().c_str(), match.GetScore());
    }

    if (results.size() > 0 && query_length > 4) {
      std::make_heap(results.begin(), results.end(), result_compare());

      std::vector<match_t> results_forward_and_backward;

      do {
        std::pop_heap(results.begin(), results.end(), result_compare());
        match_t m = results.back();
        results.pop_back();

        std::string phrase = m->GetMatchedString();

        // heuristic: stop expanding if phrase has a lower score than the worst best score
        if (best_scores.Back() > m->GetScore()) {
          TRACE("Stop backward completions score to low %d", m.GetScore());
          break;
        }

        std::reverse(phrase.begin(), phrase.end());
        // put a space at the end to avoid infix-style backward expansion, see bugtracker #161
        phrase.append(" ");

        TRACE("Do backward completion for %s (%d / %d)", m.GetMatchedString().c_str(), m.GetScore(),
              best_scores.Back());

        uint32_t last_weight = 0;
        for (auto& match : backward_completions_.GetCompletions(phrase.c_str(), number_of_results)) {
          uint32_t weight = match->GetWeight();

          if (weight < best_scores.Back()) {
            TRACE("Skip Backward, score to low %d", weight);
            // optimization: if score is falling again, results do not get better
            if (last_weight > weight) {
              TRACE("Stop Backward, no better results");
              break;
            }
            continue;
          }

          last_weight = weight;

          // accept the result
          best_scores.Put(weight);
          match->SetScore(weight);

          // reverse the matched string
          std::string matched_string = match->GetMatchedString();
          std::reverse(matched_string.begin(), matched_string.end());

          match->SetMatchedString(matched_string);

          results_forward_and_backward.push_back(match);

          TRACE("Backward Completion add: %s %d", match.GetMatchedString().c_str(), match.GetScore());
        }

        // add the forward completion as well
        results_forward_and_backward.push_back(m);
      } while (results.size() > 0);

      TRACE("Done backward completions");

      bool last_character_is_space = query[query_length - 1] == ' ';
      // check for space
      if (last_character_is_space || number_of_tokens > 1) {
        std::make_heap(results_forward_and_backward.begin(), results_forward_and_backward.end(), result_compare());

        std::string phrase = query;
        boost::trim(phrase);

        std::reverse(phrase.begin(), phrase.end());
        // put a space at the end to avoid infix-style backward expansion, see bugtracker #161
        phrase.append(" ");
        TRACE("Do backward forward completions");

        // reuse results vector
        results.clear();
        for (auto& match : backward_completions_.GetCompletions(phrase.c_str(), number_of_results)) {
          std::string matched_string = match->GetMatchedString();
          std::reverse(matched_string.begin(), matched_string.end());
          // if the original query had a space at the end, this result should as well
          if (last_character_is_space) {
            matched_string.append(" ");
          }

          uint32_t weight = match->GetWeight();
          match->SetScore(weight);
          match->SetMatchedString(matched_string);

          results.push_back(match);
          TRACE("Backward Completion from query add: %s %d", match.GetMatchedString().c_str(), match.GetScore());
        }

        if (results.size() > 0) {
          std::make_heap(results.begin(), results.end(), result_compare());

          do {
            std::pop_heap(results.begin(), results.end(), result_compare());
            match_t m = results.back();
            results.pop_back();

            std::string phrase = m->GetMatchedString();
            TRACE("Do forward from backward completion for %s (%d / %d)", m.GetMatchedString().c_str(), m.GetScore(),
                  best_scores.Back());

            // heuristic: stop expanding if phrase has a lower score than the worst best score
            if (best_scores.Back() > m->GetScore()) {
              TRACE("Stop backward forward completions scores to low %d", m.GetScore());
              break;
            }

            // match forward with this
            for (auto& match_forward :
                 forward_completions_.GetCompletions(m->GetMatchedString().c_str(), number_of_results)) {
              uint32_t weight = match_forward->GetWeight();

              if (weight < best_scores.Back()) {
                TRACE("Skip Backward forward,  score to low %d", weight);
                break;
              }

              // accept the result
              best_scores.Put(weight);
              match_forward->SetScore(weight);

              results_forward_and_backward.push_back(match_forward);

              TRACE("Backward Forward Completion add: %s %d", match_forward.GetMatchedString().c_str(),
                    match_forward.GetScore());
            }
          } while (results.size() > 0);
        }
      }

      std::swap(results, results_forward_and_backward);
    }

    std::make_heap(results.begin(), results.end(), result_compare());

    struct delegate_payload {
      explicit delegate_payload(std::vector<match_t>& r) : results(std::move(r)) {}

      std::vector<match_t> results;
      match_t last_result;
    };

    std::shared_ptr<delegate_payload> data(new delegate_payload(results));

    auto tfunc = [data]() {
      if (data->results.size()) {
        std::pop_heap(data->results.begin(), data->results.end(), result_compare());

        // de-duplicate
        while (data->last_result && data->last_result->GetMatchedString() == data->results.back()->GetMatchedString()) {
          data->results.pop_back();
          if (data->results.size() == 0) {
            return match_t();
          }

          std::pop_heap(data->results.begin(), data->results.end(), result_compare());
        }

        data->last_result = data->results.back();
        data->results.pop_back();

        return data->last_result;
      }

      return match_t();
    };

    return MatchIterator::MakeIteratorPair(tfunc);
  }

 private:
  PrefixCompletion forward_completions_;
  PrefixCompletion backward_completions_;
};

} /* namespace completion */
} /* namespace dictionary */
} /* namespace keyvi */

#endif  // KEYVI_DICTIONARY_COMPLETION_FORWARD_BACKWARD_COMPLETION_H_
