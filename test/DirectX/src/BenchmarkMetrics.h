#pragma once

#include <Windows.h>
#include <psapi.h>
#pragma comment(lib, "Psapi.lib")

#include <chrono>
#include <fstream>
#include <iostream>
#include <limits>
#include <string>

class BenchmarkMetrics {
public:
    BenchmarkMetrics(
        const std::string& apiName,
        double initMs,
        int width,
        int height,
        int drawCalls,
        int vertices,
        const std::string& outputCsv,
        double warmupSeconds = 2.0,
        double measureSeconds = 10.0)
        : _apiName(apiName),
          _initMs(initMs),
          _width(width),
          _height(height),
          _drawCalls(drawCalls),
          _vertices(vertices),
          _outputCsv(outputCsv),
          _warmupSeconds(warmupSeconds),
          _measureSeconds(measureSeconds),
          _runStart(Clock::now())
    {
        _memoryAfterInitMB = getWorkingSetMB();
    }

    void beginFrame()
    {
        _frameStart = Clock::now();
    }

    // Devuelve true cuando ya ha terminado la ventana de medición.
    bool endFrame()
    {
        const auto now = Clock::now();
        const double elapsedSeconds = std::chrono::duration<double>(now - _runStart).count();
        const double frameMs = std::chrono::duration<double, std::milli>(now - _frameStart).count();

        if (elapsedSeconds >= _warmupSeconds && elapsedSeconds < (_warmupSeconds + _measureSeconds)) {
            _frameCount++;
            _accumulatedMs += frameMs;
            if (frameMs < _minMs) _minMs = frameMs;
            if (frameMs > _maxMs) _maxMs = frameMs;
        }

        if (!_finished && elapsedSeconds >= (_warmupSeconds + _measureSeconds)) {
            _finished = true;
            writeReport();
            return true;
        }

        return false;
    }

private:
    using Clock = std::chrono::high_resolution_clock;

    static double getWorkingSetMB()
    {
        PROCESS_MEMORY_COUNTERS_EX pmc{};
        if (GetProcessMemoryInfo(
            GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&pmc),
            sizeof(pmc))) {
            return static_cast<double>(pmc.WorkingSetSize) / (1024.0 * 1024.0);
        }
        return 0.0;
    }

    void writeReport()
    {
        const double avgMs = (_frameCount > 0) ? (_accumulatedMs / static_cast<double>(_frameCount)) : 0.0;
        const double avgFps = (avgMs > 0.0) ? (1000.0 / avgMs) : 0.0;
        const double memoryEndMB = getWorkingSetMB();

        std::cout << "\n===== BENCHMARK " << _apiName << " =====\n";
        std::cout << "Resolution: " << _width << "x" << _height << "\n";
        std::cout << "Init: " << _initMs << " ms\n";
        std::cout << "Frames measured: " << _frameCount << "\n";
        std::cout << "FPS avg: " << avgFps << "\n";
        std::cout << "Frame avg: " << avgMs << " ms\n";
        std::cout << "Frame min: " << _minMs << " ms\n";
        std::cout << "Frame max: " << _maxMs << " ms\n";
        std::cout << "Memory after init: " << _memoryAfterInitMB << " MB\n";
        std::cout << "Memory at end: " << memoryEndMB << " MB\n";
        std::cout << "Draw calls: " << _drawCalls << "\n";
        std::cout << "Vertices: " << _vertices << "\n";

        std::ofstream file(_outputCsv, std::ios::app);
        if (file.tellp() == 0) {
            file << "api,resolution,init_ms,fps_avg,frame_avg_ms,frame_min_ms,frame_max_ms,mem_after_init_mb,mem_end_mb,draw_calls,vertices\n";
        }

        file << _apiName << ","
             << _width << "x" << _height << ","
             << _initMs << ","
             << avgFps << ","
             << avgMs << ","
             << _minMs << ","
             << _maxMs << ","
             << _memoryAfterInitMB << ","
             << memoryEndMB << ","
             << _drawCalls << ","
             << _vertices << "\n";
    }

    std::string _apiName;
    double _initMs = 0.0;
    int _width = 0;
    int _height = 0;
    int _drawCalls = 0;
    int _vertices = 0;
    std::string _outputCsv;
    double _warmupSeconds = 2.0;
    double _measureSeconds = 10.0;

    Clock::time_point _runStart;
    Clock::time_point _frameStart;

    int _frameCount = 0;
    double _accumulatedMs = 0.0;
    double _minMs = (std::numeric_limits<double>::max)();
    double _maxMs = 0.0;
    double _memoryAfterInitMB = 0.0;
    bool _finished = false;
};
