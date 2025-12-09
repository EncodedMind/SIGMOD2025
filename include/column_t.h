#pragma once
#include <cstdint>
#include <value_t.h>
#include <vector>

namespace columnt{

    constexpr size_t PAGE_SIZE = 8192;

    struct alignas(8) Intermediate_Page{
        valuet::value_t data[PAGE_SIZE / sizeof(valuet::value_t)];
    };

    struct column_t{
        std::vector<Intermediate_Page*> pages;
        size_t                          num_rows = 0;

        void push_back(const valuet::value_t& value);
        size_t size() const;
        const valuet::value_t& operator[](size_t idx) const;
        valuet::value_t&       operator[](size_t idx);
        void reserve(size_t new_cap);
    };

    // column_t();
}