//  Copyright (c) 2012 Thomas Heller
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <hpx/config.hpp>
#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/assert.hpp>
#include <hpx/include/util.hpp>
#include <hpx/modules/memory.hpp>

#include <cstddef>
#include <vector>

namespace jacobi {
    struct value_holder
    {
        value_holder()
          : count_(0)
        {
        }

        value_holder(std::size_t n, double init = 0.0)
          : v_(n, init)
          , count_(0)
        {
        }

        double& operator[](std::size_t i)
        {
            HPX_ASSERT(i < v_.size());
            return v_[i];
        }

        double const& operator[](std::size_t i) const
        {
            HPX_ASSERT(i < v_.size());
            return v_[i];
        }

        std::vector<double> v_;
        hpx::util::atomic_count count_;

        friend void intrusive_ptr_add_ref(value_holder* p) noexcept
        {
            ++p->count_;
        }
        friend void intrusive_ptr_release(value_holder* p) noexcept
        {
            if (0 == --p->count_)
                delete p;
        }

        template <typename Archive>
        void serialize(Archive& ar, unsigned)
        {
            ar & v_;
        }
    };

    struct row_range
    {
        std::ptrdiff_t begin_;
        std::ptrdiff_t end_;

        hpx::intrusive_ptr<value_holder> values_;

        row_range() {}

        row_range(hpx::intrusive_ptr<value_holder> values, std::ptrdiff_t b,
            std::ptrdiff_t e)
          : begin_(b)
          , end_(e)
          , values_(values)
        {
            HPX_ASSERT(end_ > begin_);
        }

        std::vector<double>::iterator begin()
        {
            HPX_ASSERT(values_);
            return values_->v_.begin() + begin_;
        }

        std::vector<double>::const_iterator begin() const
        {
            HPX_ASSERT(values_);
            return values_->v_.begin() + begin_;
        }

        std::vector<double>::iterator end()
        {
            HPX_ASSERT(values_);
            return values_->v_.begin() + end_;
        }

        std::vector<double>::const_iterator end() const
        {
            HPX_ASSERT(values_);
            return values_->v_.begin() + end_;
        }

        template <typename Archive>
        void load(Archive& ar, unsigned)
        {
            values_.reset(new value_holder());
            ar & values_->v_;
            begin_ = 0;
            end_ = static_cast<std::ptrdiff_t>(values_->v_.size());
            HPX_ASSERT(end_ > begin_);
        }

        template <typename Archive>
        void save(Archive& ar, unsigned) const
        {
            HPX_ASSERT(values_);
            std::vector<double> tmp(
                values_->v_.begin() + begin_, values_->v_.begin() + end_);
            ar & tmp;
        }

        HPX_SERIALIZATION_SPLIT_MEMBER()
    };

    inline std::vector<double>::iterator range_begin(row_range& r)
    {
        return r.begin();
    }

    inline std::vector<double>::iterator range_end(row_range& r)
    {
        return r.end();
    }

    inline std::vector<double>::const_iterator range_begin(row_range const& r)
    {
        return r.begin();
    }

    inline std::vector<double>::const_iterator range_end(row_range const& r)
    {
        return r.end();
    }
}    // namespace jacobi

#endif
