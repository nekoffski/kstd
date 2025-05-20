#pragma once

#include <algorithm>
#include <vector>

#include "Concepts.hh"

namespace kstd {

template <typename Container, typename Transformation>
requires(Iterable<Container> && Countable<Container> && std::invocable<Transformation, const typename Container::value_type&>)
auto transform(const Container& c, Transformation&& t) {
    using OutType =
      std::result_of_t<Transformation(const typename Container::value_type&)>;

    std::vector<OutType> out;
    out.reserve(c.size());
    std::transform(c.begin(), c.end(), std::back_inserter(out), t);
    return out;
}

template <typename Container, typename Condition>
requires(Iterable<Container> && Countable<Container> && std::invocable<Condition, const typename Container::value_type&>)
auto filter(const Container& c, Condition&& t) {
    using OutType = Container::value_type;

    std::vector<OutType> out;
    out.reserve(c.size());
    std::copy_if(c.begin(), c.end(), std::back_inserter(out), t);
    return out;
}

}  // namespace kstd
