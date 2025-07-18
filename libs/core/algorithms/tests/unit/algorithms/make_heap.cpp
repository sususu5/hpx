//  Copyright (c) 2015 Grant Mercer
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/algorithm.hpp>
#include <hpx/init.hpp>
#include <hpx/modules/testing.hpp>

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <string>
#include <vector>

#include "test_utils.hpp"

///////////////////////////////////////////////////////////////////////////
unsigned int seed = std::random_device{}();
std::mt19937 gen(seed);

///////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_make_heap_small1(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t len = 0;
    while (len < 15)
    {
        std::vector<std::size_t> c(len);
        std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

        hpx::make_heap(
            iterator(hpx::util::begin(c)), iterator(hpx::util::end(c)));

        HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c)), true);
        len++;
    }
}

template <typename IteratorTag>
void test_make_heap1(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    hpx::make_heap(iterator(hpx::util::begin(c)), iterator(hpx::util::end(c)));

    HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c)), true);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_small1(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::size_t len = 0;
    while (len < 15)
    {
        std::vector<std::size_t> c(len);
        std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

        hpx::make_heap(
            policy, iterator(hpx::util::begin(c)), iterator(hpx::util::end(c)));

        HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c)), true);
        len++;
    }
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap1(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), 0);

    hpx::make_heap(
        policy, iterator(hpx::util::begin(c)), iterator(hpx::util::end(c)));

    HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c)), true);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_async1(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), 0);

    hpx::future<void> test = hpx::make_heap(
        p, iterator(hpx::util::begin(c)), iterator(hpx::util::end(c)));

    test.wait();
    HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c)), true);
}

template <typename IteratorTag>
void test_make_heap1()
{
    using namespace hpx::execution;

    test_make_heap1(IteratorTag());
    test_make_heap_small1(IteratorTag());

    test_make_heap1(seq, IteratorTag());
    test_make_heap1(par, IteratorTag());
    test_make_heap1(par_unseq, IteratorTag());

    test_make_heap_small1(seq, IteratorTag());
    test_make_heap_small1(par, IteratorTag());
    test_make_heap_small1(par_unseq, IteratorTag());

    test_make_heap_async1(seq(task), IteratorTag());
    test_make_heap_async1(par(task), IteratorTag());
}

void make_heap_test1()
{
    test_make_heap1<std::random_access_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_make_heap2(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(25);
    std::iota(hpx::util::begin(c), hpx::util::end(c), 0);

    hpx::make_heap(iterator(hpx::util::begin(c)), iterator(hpx::util::end(c)),
        std::greater<std::size_t>());

    HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c),
                    std::greater<std::size_t>()),
        true);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap2(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(25);
    std::iota(hpx::util::begin(c), hpx::util::end(c), 0);

    hpx::make_heap(policy, iterator(hpx::util::begin(c)),
        iterator(hpx::util::end(c)), std::greater<std::size_t>());

    HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c),
                    std::greater<std::size_t>()),
        true);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_async2(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::test_iterator<base_iterator, IteratorTag> iterator;

    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), 0);

    hpx::future<void> test = hpx::make_heap(p, iterator(hpx::util::begin(c)),
        iterator(hpx::util::end(c)), std::greater<std::size_t>());

    test.wait();
    HPX_TEST_EQ(std::is_heap(hpx::util::begin(c), hpx::util::end(c),
                    std::greater<std::size_t>()),
        true);
}

template <typename IteratorTag>
void test_make_heap2()
{
    using namespace hpx::execution;

    test_make_heap2(IteratorTag());

    test_make_heap2(seq, IteratorTag());
    test_make_heap2(par, IteratorTag());
    test_make_heap2(par_unseq, IteratorTag());

    test_make_heap_async2(seq(task), IteratorTag());
    test_make_heap_async2(par(task), IteratorTag());
}

void make_heap_test2()
{
    test_make_heap2<std::random_access_iterator_tag>();
}

///////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_make_heap_exception(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;
    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    bool caught_exception = false;
    try
    {
        hpx::make_heap(decorated_iterator(hpx::util::begin(c),
                           []() { throw std::runtime_error("test"); }),
            decorated_iterator(hpx::util::end(c)));
        HPX_TEST(false);
    }
    catch (hpx::exception_list const&)
    {
        caught_exception = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_exception(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;
    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    bool caught_exception = false;
    try
    {
        hpx::make_heap(policy,
            decorated_iterator(hpx::util::begin(c),
                []() { throw std::runtime_error("test"); }),
            decorated_iterator(hpx::util::end(c)));
        HPX_TEST(false);
    }
    catch (hpx::exception_list const&)
    {
        caught_exception = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_exception);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_exception_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    bool caught_exception = false;
    bool returned_from_algorithm = false;
    try
    {
        hpx::future<void> f = hpx::make_heap(p,
            decorated_iterator(hpx::util::begin(c),
                []() { throw std::runtime_error("test"); }),
            decorated_iterator(hpx::util::end(c)));
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (hpx::exception_list const&)
    {
        caught_exception = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }
    HPX_TEST(caught_exception);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_make_heap_exception()
{
    using namespace hpx::execution;

    test_make_heap_exception(IteratorTag());

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_make_heap_exception(seq, IteratorTag());
    test_make_heap_exception(par, IteratorTag());

    test_make_heap_exception_async(seq(task), IteratorTag());
    test_make_heap_exception_async(par(task), IteratorTag());
}

void make_heap_exception_test()
{
    test_make_heap_exception<std::random_access_iterator_tag>();
}

//////////////////////////////////////////////////////////////////////////////
template <typename IteratorTag>
void test_make_heap_bad_alloc(IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(100007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    bool caught_bad_alloc = false;
    try
    {
        hpx::make_heap(decorated_iterator(hpx::util::begin(c),
                           []() { throw std::bad_alloc(); }),
            decorated_iterator(hpx::util::end(c)));
        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_bad_alloc(ExPolicy&& policy, IteratorTag)
{
    static_assert(hpx::is_execution_policy<ExPolicy>::value,
        "hpx::is_execution_policy<ExPolicy>::value");

    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(100007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    bool caught_bad_alloc = false;
    try
    {
        hpx::make_heap(policy,
            decorated_iterator(
                hpx::util::begin(c), []() { throw std::bad_alloc(); }),
            decorated_iterator(hpx::util::end(c)));
        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
}

template <typename ExPolicy, typename IteratorTag>
void test_make_heap_bad_alloc_async(ExPolicy&& p, IteratorTag)
{
    typedef std::vector<std::size_t>::iterator base_iterator;
    typedef test::decorated_iterator<base_iterator, IteratorTag>
        decorated_iterator;

    std::vector<std::size_t> c(10007);
    std::iota(hpx::util::begin(c), hpx::util::end(c), gen());

    bool caught_bad_alloc = false;
    bool returned_from_algorithm = false;
    try
    {
        hpx::future<void> f = hpx::make_heap(p,
            decorated_iterator(
                hpx::util::begin(c), []() { throw std::bad_alloc(); }),
            decorated_iterator(hpx::util::end(c)));
        returned_from_algorithm = true;
        f.get();

        HPX_TEST(false);
    }
    catch (std::bad_alloc const&)
    {
        caught_bad_alloc = true;
    }
    catch (...)
    {
        HPX_TEST(false);
    }

    HPX_TEST(caught_bad_alloc);
    HPX_TEST(returned_from_algorithm);
}

template <typename IteratorTag>
void test_make_heap_bad_alloc()
{
    using namespace hpx::execution;

    test_make_heap_bad_alloc(IteratorTag());

    // If the execution policy object is of type vector_execution_policy,
    // std::terminate shall be called. therefore we do not test exceptions
    // with a vector execution policy
    test_make_heap_bad_alloc(seq, IteratorTag());
    test_make_heap_bad_alloc(par, IteratorTag());

    test_make_heap_bad_alloc_async(seq(task), IteratorTag());
    test_make_heap_bad_alloc_async(par(task), IteratorTag());
}

void make_heap_bad_alloc_test()
{
    test_make_heap_bad_alloc<std::random_access_iterator_tag>();
}

int hpx_main(hpx::program_options::variables_map& vm)
{
    if (vm.count("seed"))
        seed = vm["seed"].as<unsigned int>();

    std::cout << "using seed: " << seed << std::endl;
    gen.seed(seed);

    make_heap_test1();
    make_heap_test2();
    make_heap_exception_test();
    make_heap_bad_alloc_test();

    return hpx::local::finalize();
}

int main(int argc, char* argv[])
{
    using namespace hpx::program_options;
    options_description desc_commandline(
        "Usage: " HPX_APPLICATION_STRING " [options]");

    desc_commandline.add_options()("seed,s", value<unsigned int>(),
        "the random number generator seed to use for this run");

    hpx::local::init_params init_args;
    init_args.desc_cmdline = desc_commandline;

    HPX_TEST_EQ_MSG(hpx::local::init(hpx_main, argc, argv, init_args), 0,
        "HPX main exited with a non-zero status");

    return hpx::util::report_errors();
}
