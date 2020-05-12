//
// Created by peiming on 3/24/20.
//
#include <gtest/gtest.h>
#include <llvm/Support/CommandLine.h>

#include "aser/Util/Log.h"

using namespace aser;
using namespace llvm;

int main(int argc, char** argv) {
    logger::LoggingConfig config;
    config.enableFile = false;
    config.enableTerminal = true;
    config.level = spdlog::level::info;
    logger::init(config);

    ::testing::InitGoogleTest(&argc, argv);

    // call it after init google test, as google test will consume and delete argument from argument list
    llvm::cl::ParseCommandLineOptions(argc, argv);

    return RUN_ALL_TESTS();
}