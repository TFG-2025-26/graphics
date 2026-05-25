#pragma once

#include <Windows.h>
#include <psapi.h>

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>

class FluxBenchmarkMetrics {
public:
    FluxBenchmarkMetrics(
        const std::string& backendName,
        double sceneLoadMs,
        double memoryAfterSceneLoadMB,
        const std::string& csvPath)
        : _backendName(backendName),
          _sceneLoadMs(sceneLoadMs),
          _memoryAfterSceneLoadMB(memoryAfterSceneLoadMB),
          _csvPath(csvPath)
    {
        _start = Clock::now();
        _lastFrameStart = _start;
    }

    void beginFrame()
    {
        _lastFrameStart = Clock::now();
    }

    bool endFrame()
    {
        const auto now = Clock::now();
        const double frameMs = std::chrono::duration<double, std::milli>(now - _lastFrameStart).count();
        const double elapsedSeconds = std::chrono::duration<double>(now - _start).count();

        if (elapsedSeconds >= WarmupSeconds && elapsedSeconds < WarmupSeconds + MeasureSeconds) {
            _measuredFrames++;
            _accumulatedMs += frameMs;
            _minFrameMs = std::min<double>(_minFrameMs, frameMs);
            _maxFrameMs = std::max<double>(_maxFrameMs, frameMs);
        }

        if (!_finished && elapsedSeconds >= WarmupSeconds + MeasureSeconds) {
            _finished = true;
            _memoryAtEndMB = getProcessMemoryMB();
            writeResults();
            return true;
        }

        return false;
    }

    static double getProcessMemoryMB()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        if (!GetProcessMemoryInfo(
            GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
            sizeof(pmc))) {
            return 0.0;
        }

        return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
    }

private:
    using Clock = std::chrono::high_resolution_clock;

    void writeResults() const
    {
        const double avgFrameMs = _measuredFrames > 0 ? _accumulatedMs / static_cast<double>(_measuredFrames) : 0.0;
        const double avgFps = avgFrameMs > 0.0 ? 1000.0 / avgFrameMs : 0.0;

        std::cout << "\n===== FLUX ENGINE BENCHMARK: " << _backendName << " =====\n";
        std::cout << "Scene load: " << _sceneLoadMs << " ms\n";
        std::cout << "Frames measured: " << _measuredFrames << "\n";
        std::cout << "FPS avg: " << avgFps << "\n";
        std::cout << "Frame avg: " << avgFrameMs << " ms\n";
        std::cout << "Frame min: " << _minFrameMs << " ms\n";
        std::cout << "Frame max: " << _maxFrameMs << " ms\n";
        std::cout << "Memory after scene load: " << _memoryAfterSceneLoadMB << " MB\n";
        std::cout << "Memory at end: " << _memoryAtEndMB << " MB\n";
        std::cout << "CSV: " << _csvPath << "\n";

        std::ofstream csv(_csvPath, std::ios::trunc);
        csv << std::fixed << std::setprecision(4);
        csv << "backend,scene_load_ms,fps_avg,frame_avg_ms,frame_min_ms,frame_max_ms,memory_after_scene_load_mb,memory_at_end_mb,frames_measured\n";
        csv << _backendName << ','
            << _sceneLoadMs << ','
            << avgFps << ','
            << avgFrameMs << ','
            << _minFrameMs << ','
            << _maxFrameMs << ','
            << _memoryAfterSceneLoadMB << ','
            << _memoryAtEndMB << ','
            << _measuredFrames << '\n';
    }

    static constexpr double WarmupSeconds = 2.0;
    static constexpr double MeasureSeconds = 10.0;

    std::string _backendName;
    double _sceneLoadMs = 0.0;
    double _memoryAfterSceneLoadMB = 0.0;
    double _memoryAtEndMB = 0.0;
    std::string _csvPath;

    Clock::time_point _start{};
    Clock::time_point _lastFrameStart{};

    int _measuredFrames = 0;
    double _accumulatedMs = 0.0;
    double _minFrameMs = (std::numeric_limits<double>::max)();
    double _maxFrameMs = 0.0;
    bool _finished = false;
};
