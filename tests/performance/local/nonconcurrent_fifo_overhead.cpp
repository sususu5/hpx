////////////////////////////////////////////////////////////////////////////////
//  Copyright (c) 2014 Bryce Adelstein-Lelbach
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
////////////////////////////////////////////////////////////////////////////////

#include <hpx/concurrency/barrier.hpp>
#include <hpx/modules/concurrency.hpp>
#include <hpx/modules/format.hpp>
#include <hpx/modules/program_options.hpp>
#include <hpx/modules/timing.hpp>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "worker_timed.hpp"

char const* benchmark_name = "Serial FIFO Overhead";

using hpx::program_options::command_line_parser;
using hpx::program_options::notify;
using hpx::program_options::options_description;
using hpx::program_options::store;
using hpx::program_options::value;
using hpx::program_options::variables_map;

namespace compat = hpx::compat;
using hpx::chrono::high_resolution_timer;

///////////////////////////////////////////////////////////////////////////////
std::uint64_t threads = 1;
std::uint64_t blocksize = 10000;
std::uint64_t iterations = 2000000;
bool header = true;

///////////////////////////////////////////////////////////////////////////////
std::string format_build_date()
{
    std::chrono::time_point<std::chrono::system_clock> now =
        std::chrono::system_clock::now();

    std::time_t current_time = std::chrono::system_clock::to_time_t(now);

    std::string ts = std::ctime(&current_time);
    ts.resize(ts.size() - 1);    // remove trailing '\n'
    return ts;
}

///////////////////////////////////////////////////////////////////////////////
void print_results(variables_map& vm, std::pair<double, double> elapsed_control,
    std::pair<double, double> elapsed_lockfree)
{
    if (header)
    {
        std::cout << "# BENCHMARK: " << benchmark_name << "\n";

        std::cout << "# VERSION: " << format_build_date() << "\n"
                  << "#\n";

        // Note that if we change the number of fields above, we have to
        // change the constant that we add when printing out the field # for
        // performance counters below (e.g. the last_index part).
        std::cout
            << "## 0:ITER:Iterations per OS-thread - Independent Variable\n"
               "## 1:BSIZE:Maximum Queue Depth - Independent Variable\n"
               "## 2:OSTHRDS:OS-thread - Independent Variable\n"
               "## 3:WTIME_CTL_PUSH:Total Walltime/Push for "
               "std::vector [nanoseconds]\n"
               "## 4:WTIME_CTL_POP:Total Walltime/Pop for "
               "std::vector [nanoseconds]\n"
               "## 5:WTIME_LF_PUSH:Total Walltime/Push for "
               "hpx::lockfree::queue [nanoseconds]\n"
               "## 6:WTIME_LF_POP:Total Walltime/Pop for "
               "hpx::lockfree::queue [nanoseconds]\n";
    }

    if (iterations != 0)
        hpx::util::format_to(std::cout,
            "{} {} {} {:.14g} {:.14g} {:.14g} {:.14g}\n", iterations, blocksize,
            threads, (elapsed_lockfree.first / (threads * iterations)) * 1e9,
            (elapsed_lockfree.second / (threads * iterations)) * 1e9,
            (elapsed_control.first / (threads * iterations)) * 1e9,
            (elapsed_control.second / (threads * iterations)) * 1e9);
    else
        hpx::util::format_to(std::cout,
            "{} {} {} {:.14g} {:.14g} {:.14g} {:.14g}\n", iterations, blocksize,
            threads, elapsed_lockfree.first * 1e9 elapsed_lockfree.second * 1e9,
            elapsed_control.first * 1e9, elapsed_control.second * 1e9);
}

///////////////////////////////////////////////////////////////////////////////
template <typename T>
void push(std::vector<T>& fifo, T& seed)
{
    fifo.push_back(seed);
}

template <typename Fifo, typename T>
void push(Fifo& fifo, T& seed)
{
    fifo.push(seed);
}

template <typename T>
void pop(std::vector<T>& fifo)
{
    fifo.pop_back();
}

template <typename Fifo>
void pop(Fifo& fifo)
{
    typename Fifo::value_type t;
    fifo.pop(t);
}

template <typename Fifo>
std::pair<double, double> bench_fifo(Fifo& fifo, std::uint64_t local_iterations)
{
    ///////////////////////////////////////////////////////////////////////////
    // Push.
    typename Fifo::value_type seed;

    std::pair<double, double> elapsed(0.0, 0.0);

    // Start the clock.
    high_resolution_timer t;

    for (std::uint64_t block = 0; block < (local_iterations / blocksize);
        ++block)
    {
        // Restart the clock.
        t.restart();

        for (std::uint64_t i = 0; i < blocksize; ++i)
        {
            push(fifo, seed);
        }

        elapsed.first += t.elapsed();

        ///////////////////////////////////////////////////////////////////////
        // Pop.

        // Restart the clock.
        t.restart();

        for (std::uint64_t i = 0; i < blocksize; ++i)
        {
            pop(fifo);
        }

        elapsed.second += t.elapsed();
    }

    return elapsed;
}

///////////////////////////////////////////////////////////////////////////////
void perform_iterations(hpx::util::barrier& b,
    std::pair<double, double>& elapsed_control,
    std::pair<double, double>& elapsed_lockfree)
{
    {
        std::vector<std::uint64_t> fifo;
        fifo.reserve(blocksize);

        // Warmup.
        bench_fifo(fifo, blocksize);

        elapsed_control = bench_fifo(fifo, blocksize);
    }

    {
        hpx::lockfree::queue<std::uint64_t> fifo(blocksize);

        // Warmup.
        bench_fifo(fifo, blocksize);

        elapsed_lockfree = bench_fifo(fifo, iterations);
    }
}

///////////////////////////////////////////////////////////////////////////////
int app_main(variables_map& vm)
{
    std::vector<std::pair<double, double>> elapsed_control(
        threads, std::pair<double, double>(0.0, 0.0));
    std::vector<std::pair<double, double>> elapsed_lockfree(
        threads, std::pair<double, double>(0.0, 0.0));
    std::vector<std::thread> workers;
    hpx::util::barrier b(threads);

    for (std::uint32_t i = 0; i != threads; ++i)
        workers.push_back(std::thread(perform_iterations, std::ref(b),
            std::ref(elapsed_control[i]), std::ref(elapsed_lockfree[i])));

    for (std::thread& thread : workers)
    {
        if (thread.joinable())
            thread.join();
    }

    std::pair<double, double> total_elapsed_control(0.0, 0.0);
    std::pair<double, double> total_elapsed_lockfree(0.0, 0.0);

    for (std::uint64_t i = 0; i < elapsed_control.size(); ++i)
    {
        total_elapsed_control.first += elapsed_control[i].first;
        total_elapsed_control.second += elapsed_control[i].second;

        total_elapsed_lockfree.first += elapsed_lockfree[i].first;
        total_elapsed_lockfree.second += elapsed_lockfree[i].second;
    }

    // Print out the results.
    print_results(vm, total_elapsed_control, total_elapsed_lockfree);

    return 0;
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    ///////////////////////////////////////////////////////////////////////////
    // Parse command line.
    variables_map vm;

    options_description cmdline("Usage: serial_fifo_overhead [options]");

    cmdline.add_options()("help,h", "print out program usage (this message)")

        ("threads,t", value<std::uint64_t>(&threads)->default_value(1),
            "number of threads to use")

            ("iterations",
                value<std::uint64_t>(&iterations)->default_value(2000000),
                "number of iterations to perform (most be divisible by block "
                "size)")

                ("blocksize",
                    value<std::uint64_t>(&blocksize)->default_value(10000),
                    "size of each block")

                    ("no-header", "do not print out the header");

    store(command_line_parser(argc, argv).options(cmdline).run(), vm);

    notify(vm);

    // Print help screen.
    if (vm.count("help"))
    {
        std::cout << cmdline;
        return 0;
    }

    if (iterations % blocksize)
        throw std::invalid_argument(
            "iterations must be cleanly divisible by blocksize\n");

    if (vm.count("no-header"))
        header = false;

    return app_main(vm);
}
