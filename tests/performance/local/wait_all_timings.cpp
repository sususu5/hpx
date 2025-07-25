//  Copyright (c) 2016-2025 Hartmut Kaiser
//
//  SPDX-License-Identifier: BSL-1.0
//  Distributed under the Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#include <hpx/config.hpp>
#if !defined(HPX_COMPUTE_DEVICE_CODE)
#include <hpx/chrono.hpp>
#include <hpx/format.hpp>
#include <hpx/future.hpp>
#include <hpx/init.hpp>
#include <hpx/modules/testing.hpp>
#include <hpx/program_options.hpp>

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

///////////////////////////////////////////////////////////////////////////////
std::vector<hpx::future<void>> create_tasks(
    std::size_t num_tasks, std::size_t delay)
{
    std::vector<hpx::future<void>> tasks;
    tasks.reserve(num_tasks);
    for (std::size_t i = 0; i != num_tasks; ++i)
    {
        if (delay == 0)
        {
            tasks.push_back(hpx::make_ready_future());
        }
        else
        {
            tasks.push_back(
                hpx::make_ready_future_after(std::chrono::microseconds(delay)));
        }
    }
    return tasks;
}

double wait_tasks(std::size_t num_samples, std::size_t num_tasks,
    std::size_t num_chunks, std::size_t delay)
{
    std::size_t num_chunk_tasks = ((num_tasks + num_chunks) / num_chunks) - 1;
    std::size_t last_num_chunk_tasks =
        num_tasks - (num_chunks - 1) * num_chunk_tasks;

    double result = 0;

    for (std::size_t k = 0; k != num_samples; ++k)
    {
        std::vector<std::vector<hpx::future<void>>> chunks;
        chunks.reserve(num_chunks);
        for (std::size_t c = 0; c != num_chunks - 1; ++c)
        {
            chunks.push_back(create_tasks(num_chunk_tasks, delay));
        }
        chunks.push_back(create_tasks(last_num_chunk_tasks, delay));

        std::vector<hpx::future<void>> chunk_results;
        chunk_results.reserve(num_chunks);

        // wait of tasks in chunks
        hpx::chrono::high_resolution_timer t;
        if (num_chunks == 1)
        {
            hpx::wait_all(chunks[0]);
        }
        else
        {
            for (std::size_t c = 0; c != num_chunks; ++c)
            {
                chunk_results.push_back(
                    hpx::async([&chunks, c]() { hpx::wait_all(chunks[c]); }));
            }
            hpx::wait_all(chunk_results);
        }
        result += t.elapsed();
    }

    return result / static_cast<double>(num_samples);
}

///////////////////////////////////////////////////////////////////////////////
int hpx_main(hpx::program_options::variables_map& vm)
{
    std::size_t num_samples = 1000;
    std::size_t num_tasks = 100;
    std::size_t num_chunks = 1;
    std::size_t delay = 0;
    bool header = true;

    if (vm.count("no-header"))
        header = false;
    if (vm.count("samples"))
        num_samples = vm["samples"].as<std::size_t>();
    if (vm.count("futures"))
        num_tasks = vm["futures"].as<std::size_t>();
    if (vm.count("chunks"))
        num_chunks = vm["chunks"].as<std::size_t>();
    if (vm.count("delay"))
        delay = vm["delay"].as<std::size_t>();

    if (num_chunks == 0)
        num_chunks = 1;

    // wait for all of the tasks sequentially
    double elapsed_seq = wait_tasks(num_samples, num_tasks, 1, delay);

    // wait of tasks in chunks
    double elapsed_chunks = 0;
    if (num_chunks != 1)
        elapsed_chunks = wait_tasks(num_samples, num_tasks, num_chunks, delay);

    if (header)
    {
        std::cout
            << "Tasks,Chunks,Delay[s],Total Walltime[s],Walltime per Task[s]"
            << std::endl;
    }

    std::string const tasks_str = hpx::util::format("{}", num_tasks);
    std::string const chunks_str = hpx::util::format("{}", num_chunks);
    std::string const delay_str = hpx::util::format("{}", delay);

    hpx::util::format_to(std::cout, "{:10},{:10},{:10},{:10},{:10.12}\n",
        tasks_str, std::string("1"), delay_str, elapsed_seq,
        elapsed_seq / static_cast<double>(num_tasks))
        << std::endl;
    hpx::util::print_cdash_timing(
        "WaitAll", elapsed_seq / static_cast<double>(num_tasks));

    if (num_chunks != 1)
    {
        hpx::util::format_to(std::cout,
            "{:10},{:10},{:10},{:10},{:10.12},{:10.12}\n", tasks_str,
            chunks_str, delay_str, elapsed_chunks,
            elapsed_chunks / static_cast<double>(num_tasks))
            << std::endl;
        hpx::util::print_cdash_timing(
            "WaitAllChunks", elapsed_chunks / static_cast<double>(num_tasks));
    }
    return hpx::local::finalize();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[])
{
    namespace po = hpx::program_options;

    // Configure application-specific options.
    po::options_description cmdline(
        "usage: " HPX_APPLICATION_STRING " [options]");
    cmdline.add_options()("samples,s",
        po::value<std::size_t>()->default_value(1000),
        "number of tasks to concurrently wait for (default: 1000)")("futures,f",
        po::value<std::size_t>()->default_value(100),
        "number of tasks to concurrently wait for (default: 100)")("chunks,c",
        po::value<std::size_t>()->default_value(1),
        "number of chunks to split tasks into (default: 1)")("delay,d",
        po::value<std::size_t>()->default_value(0),
        "number of iterations in the delay loop")("no-header,n",
        po::value<bool>()->default_value(true),
        "do not print out the csv header row");

    // Initialize and run HPX.
    hpx::local::init_params init_args;
    init_args.desc_cmdline = cmdline;

    return hpx::local::init(hpx_main, argc, argv, init_args);
}
#endif
