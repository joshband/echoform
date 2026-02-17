// main.cpp
//
// QA harness test runner for Echoform

#include "echoform_adapter.h"
#include "../external/qa_harness/scenario_engine/scenario_executor.h"
#include "../external/qa_harness/scenario_engine/scenario_loader.h"
#include "../external/qa_harness/scenario_engine/test_suite_loader.h"
#include "../external/qa_harness/scenario_engine/test_suite_executor.h"
#include "../external/qa_harness/scenario_engine/invariant_evaluator.h"
#include "../external/qa_harness/runners/in_process_runner.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <cstdlib>
#include <filesystem>

namespace {

// Factory function for creating Echoform adapter
std::unique_ptr<qa::DspUnderTest> createEchoformDut()
{
    return std::make_unique<echoform::EchoformAdapter>();
}

// QARunnerFactory that creates an InProcessRunner from a DutFactory
qa::scenario::QARunnerFactory makeInProcessRunnerFactory()
{
    return [](qa::DutFactory dutFactory) -> std::unique_ptr<qa::QARunner> {
        return std::make_unique<qa::InProcessRunner>(dutFactory);
    };
}

std::string suiteStatusLabel(const qa::scenario::TestSuiteResult& result)
{
    if (result.errorCount > 0)
        return "ERROR";
    if (result.failCount > 0)
        return "FAIL";
    if (result.warnCount > 0)
        return "WARN";
    if (result.totalScenarios > 0 && result.skipCount == result.totalScenarios)
        return "SKIP";
    return "PASS";
}

struct TopFinding
{
    std::string severity;
    std::string scenarioId;
    std::string message;
};

bool findTopFinding(const qa::scenario::TestSuiteResult& result, TopFinding& finding)
{
    for (const auto& scenario : result.scenarioResults)
    {
        if (scenario.status == qa::scenario::ScenarioResult::Status::ERROR)
        {
            finding.severity = "ERROR";
            finding.scenarioId = scenario.scenarioId;
            finding.message = scenario.errorMessage.empty() ? "scenario execution error" : scenario.errorMessage;
            return true;
        }
    }

    for (const auto& scenario : result.scenarioResults)
    {
        if (scenario.status == qa::scenario::ScenarioResult::Status::FAIL)
        {
            finding.severity = "FAIL";
            finding.scenarioId = scenario.scenarioId;
            if (!scenario.hardFailures.empty())
                finding.message = scenario.hardFailures.front();
            else
                finding.message = "hard invariant failure";
            return true;
        }
    }

    for (const auto& scenario : result.scenarioResults)
    {
        if (scenario.status == qa::scenario::ScenarioResult::Status::WARN)
        {
            finding.severity = "WARN";
            finding.scenarioId = scenario.scenarioId;
            if (!scenario.softWarnings.empty())
                finding.message = scenario.softWarnings.front();
            else
                finding.message = "soft warning present";
            return true;
        }
    }

    return false;
}

void printSuiteSummaryBlock(const qa::scenario::TestSuiteResult& result,
                           const std::filesystem::path& reportPath)
{
    std::cout << "Suite: " << result.suiteId << "  Status: " << suiteStatusLabel(result) << "\n";
    std::cout << "Total: " << result.totalScenarios
              << "  Pass: " << result.passCount
              << "  Warn: " << result.warnCount
              << "  Fail: " << result.failCount
              << "  Error: " << result.errorCount
              << "  Skip: " << result.skipCount << "\n";

    TopFinding finding;
    if (findTopFinding(result, finding))
        std::cout << "Top finding: " << finding.scenarioId << " (" << finding.severity << ") " << finding.message << "\n";
    else
        std::cout << "Top finding: none\n";

    std::cout << "Report: " << reportPath << "\n";
}

int runScenario(const std::string& scenarioPath, bool captureBaseline, bool compareBaseline, bool enableProfiling)
{
    std::cout << "Running scenario: " << scenarioPath << "\n";

    // Load scenario
    auto loadResult = qa::scenario::loadScenarioFile(scenarioPath);
    if (!loadResult.ok)
    {
        std::cerr << "ERROR: Failed to load scenario\n";
        for (const auto& error : loadResult.errors)
            std::cerr << "  - " << error << "\n";
        return 1;
    }

    // Create execution config
    qa::scenario::ExecutionConfig config;
    config.sampleRate = 48000;
    config.blockSize = 512;
    config.numChannels = 2;
    config.outputDir = "qa_output";
    config.enableProfiling = enableProfiling;

    // Baseline configuration
    config.captureBaseline = captureBaseline;
    config.compareToBaseline = compareBaseline;
    config.baselineDir = "baselines";
    config.baselineTolerance = 5.0;  // 5% tolerance
    config.baselineVersion = "v1.0.0";

    // Create scenario executor
    qa::scenario::ScenarioExecutor executor(
        makeInProcessRunnerFactory(),
        createEchoformDut,
        config
    );

    // Execute scenario
    qa::scenario::ScenarioResult result = executor.execute(loadResult.scenario);

    // Evaluate invariants
    qa::scenario::InvariantEvaluator evaluator;
    evaluator.evaluateInto(loadResult.scenario, result);

    // Report results
    std::cout << "\n=== Scenario Results ===\n";
    std::cout << "Dry path: " << result.dryPath << "\n";
    std::cout << "Wet path: " << result.wetPath << "\n";
    std::cout << "Status: ";
    switch (result.status)
    {
        case qa::scenario::ScenarioResult::Status::PASS:
            std::cout << "PASS\n";
            break;
        case qa::scenario::ScenarioResult::Status::WARN:
            std::cout << "WARN\n";
            break;
        case qa::scenario::ScenarioResult::Status::FAIL:
            std::cout << "FAIL\n";
            break;
        case qa::scenario::ScenarioResult::Status::SKIP:
            std::cout << "SKIP (" << result.skipReason << ")\n";
            break;
        case qa::scenario::ScenarioResult::Status::ERROR:
            std::cout << "ERROR (" << result.errorMessage << ")\n";
            break;
    }

    if (!result.hardFailures.empty())
    {
        std::cout << "\nHard Failures:\n";
        for (const auto& failure : result.hardFailures)
            std::cout << "  - " << failure << "\n";
    }

    if (!result.softWarnings.empty())
    {
        std::cout << "\nSoft Warnings:\n";
        for (const auto& warning : result.softWarnings)
            std::cout << "  - " << warning << "\n";
    }

    std::cout << "\nInvariant Results:\n";
    for (const auto& invResult : result.invariantResults)
    {
        std::cout << "  " << invResult.metric << ": ";
        if (invResult.passed)
            std::cout << "PASS (value=" << invResult.measuredValue << ")\n";
        else
            std::cout << "FAIL (value=" << invResult.measuredValue << ")\n";
    }

    return (result.status == qa::scenario::ScenarioResult::Status::PASS ||
            result.status == qa::scenario::ScenarioResult::Status::WARN) ? 0 : 1;
}

int runTestSuite(const std::string& suitePath, bool captureBaseline, bool compareBaseline, bool enableProfiling)
{
    std::cout << "Running test suite: " << suitePath << "\n";

    // Determine scenario directory (assume sibling to suite file)
    std::filesystem::path scenarioDir = std::filesystem::path(suitePath).parent_path();

    // Load and resolve test suite
    auto resolvedSuite = qa::scenario::loadAndResolveTestSuite(suitePath, scenarioDir);
    if (!resolvedSuite.ok)
    {
        std::cerr << "ERROR: Failed to load test suite\n";
        for (const auto& error : resolvedSuite.errors)
            std::cerr << "  - " << error << "\n";
        return 1;
    }

    // Create execution config
    qa::scenario::ExecutionConfig config;
    config.sampleRate = 48000;
    config.blockSize = 512;
    config.numChannels = 2;
    config.outputDir = "qa_output";
    config.enableProfiling = enableProfiling;

    // Baseline configuration
    config.captureBaseline = captureBaseline;
    config.compareToBaseline = compareBaseline;
    config.baselineDir = "baselines";
    config.baselineTolerance = 5.0;  // 5% tolerance
    config.baselineVersion = "v1.0.0";

    // Create scenario executor
    qa::scenario::ScenarioExecutor scenarioExecutor(
        makeInProcessRunnerFactory(),
        createEchoformDut,
        config
    );

    // Create suite executor
    qa::scenario::TestSuiteExecutor suiteExecutor(scenarioExecutor);

    // Execute suite (pass config for baseline support)
    qa::scenario::TestSuiteResult result = suiteExecutor.execute(
        resolvedSuite.suite,
        resolvedSuite.scenarios,
        &config
    );

    // Report results
    printSuiteSummaryBlock(result, config.outputDir / "report" / "summary.md");

    if (result.stoppedEarly)
        std::cout << "\n(Stopped early due to failure)\n";

    return result.passed ? 0 : 1;
}

int runDiscoverSuite(const std::string& rootDir, bool captureBaseline, bool compareBaseline,
                     bool enableProfiling)
{
    std::cout << "Auto-discovering scenarios in: " << rootDir << "\n";

    // Walk subdirectories since echoform uses nested scenario layout
    // discoverSuite() is non-recursive, so we call it per subdirectory
    std::vector<qa::scenario::ScenarioSpec> allScenarios;
    int dirCount = 0;

    for (const auto& entry : std::filesystem::directory_iterator(rootDir))
    {
        if (!entry.is_directory())
            continue;

        ++dirCount;
        auto subSuite = qa::scenario::discoverSuite(entry.path());
        if (subSuite.ok)
        {
            for (auto& scenario : subSuite.scenarios)
                allScenarios.push_back(std::move(scenario));
        }
    }

    // Also check top-level .json files
    auto topLevel = qa::scenario::discoverSuite(std::filesystem::path(rootDir));
    if (topLevel.ok)
    {
        for (auto& scenario : topLevel.scenarios)
            allScenarios.push_back(std::move(scenario));
    }

    if (allScenarios.empty())
    {
        std::cerr << "ERROR: No scenarios found in " << rootDir << "\n";
        return 1;
    }

    // Sort by scenario ID for deterministic ordering
    std::sort(allScenarios.begin(), allScenarios.end(),
              [](const auto& a, const auto& b) { return a.id < b.id; });

    std::cout << "Discovered " << allScenarios.size() << " scenarios across " << dirCount
              << " directories\n";

    // Build a synthetic suite
    qa::scenario::TestSuite suite;
    suite.id = "auto_discovered";
    suite.name = "Auto-Discovered Suite";
    for (const auto& s : allScenarios)
        suite.scenarioIds.push_back(s.id);

    // Create execution config
    qa::scenario::ExecutionConfig config;
    config.sampleRate = 48000;
    config.blockSize = 512;
    config.numChannels = 2;
    config.outputDir = "qa_output";
    config.enableProfiling = enableProfiling;
    config.captureBaseline = captureBaseline;
    config.compareToBaseline = compareBaseline;
    config.baselineDir = "baselines";
    config.baselineTolerance = 5.0;
    config.baselineVersion = "v1.0.0";

    qa::scenario::ScenarioExecutor scenarioExecutor(makeInProcessRunnerFactory(), createEchoformDut,
                                                    config);

    qa::scenario::TestSuiteExecutor suiteExecutor(scenarioExecutor);

    qa::scenario::TestSuiteResult result =
        suiteExecutor.execute(suite, allScenarios, &config);

    printSuiteSummaryBlock(result, config.outputDir / "report" / "summary.md");

    return result.passed ? 0 : 1;
}

void printUsage(const char* programName)
{
    std::cout << "Usage:\n";
    std::cout << "  " << programName << " <scenario.json>          Run single scenario\n";
    std::cout << "  " << programName << " <suite.json>             Run test suite\n";
    std::cout << "  " << programName << " --discover <dir>         Auto-discover scenarios\n";
    std::cout << "  " << programName << "                          Run smoke test\n";
    std::cout << "\nOptions:\n";
    std::cout << "  --capture-baseline                   Capture metric baselines\n";
    std::cout << "  --compare-baseline                   Compare to existing baselines\n";
    std::cout << "  --enable-profiling                   Enable performance profiling\n";
    std::cout << "\nExamples:\n";
    std::cout << "  " << programName << " suite.json --capture-baseline\n";
    std::cout << "  " << programName << " suite.json --compare-baseline\n";
    std::cout << "  " << programName << " perf_suite.json --enable-profiling\n";
    std::cout << "  " << programName << " --discover scenarios/echoform/\n";
}

} // namespace

int main(int argc, char** argv)
{
    if (argc > 5)
    {
        printUsage(argv[0]);
        return 1;
    }

    try
    {
        // Parse optional flags
        bool captureBaseline = false;
        bool compareBaseline = false;
        bool enableProfiling = false;
        bool discoverMode = false;
        std::string path;

        for (int i = 1; i < argc; ++i)
        {
            std::string arg(argv[i]);
            if (arg == "--capture-baseline")
            {
                captureBaseline = true;
            }
            else if (arg == "--compare-baseline")
            {
                compareBaseline = true;
            }
            else if (arg == "--enable-profiling")
            {
                enableProfiling = true;
            }
            else if (arg == "--discover")
            {
                discoverMode = true;
            }
            else if (arg == "--help" || arg == "-h")
            {
                printUsage(argv[0]);
                return 0;
            }
            else
            {
                path = arg;
            }
        }

        if (discoverMode)
        {
            if (path.empty())
            {
                std::cerr << "ERROR: --discover requires a directory path\n";
                printUsage(argv[0]);
                return 1;
            }
            return runDiscoverSuite(path, captureBaseline, compareBaseline, enableProfiling);
        }

        if (!path.empty())
        {
            // Detect if suite or scenario based on filename
            if (path.find("suite") != std::string::npos)
                return runTestSuite(path, captureBaseline, compareBaseline, enableProfiling);
            else
                return runScenario(path, captureBaseline, compareBaseline, enableProfiling);
        }
        else
        {
            // Default: run smoke test
            const std::string smokeTest = "scenarios/echoform/smoke_test.json";
            std::cout << "No arguments provided, running default smoke test\n";
            return runScenario(smokeTest, captureBaseline, compareBaseline, enableProfiling);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "EXCEPTION: " << e.what() << "\n";
        return 1;
    }

    return 0;
}
