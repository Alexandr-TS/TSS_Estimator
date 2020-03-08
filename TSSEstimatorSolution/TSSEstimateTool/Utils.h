#pragma once

#include <algorithm>
#include <cassert>
#include <cstring>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <future>
#include <iomanip>
#include <iostream>
#include <map>
#include <numeric>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

optional<string> GetValueAfterKeyInCSVRow(const string& row, const string& key);

bool CheckRowIsData(const string& row);

bool ProcessForHR(const string& row, vector<int>& heart_rate_series);

struct ActivityData {
    double total_elapsed_time = -1.0;
    double tss = -1.0;
};

bool ProcessForTSS(const string& row, ActivityData& activity_data);

optional<double> ProcessForElapsedTime(const string& row);

struct FileData {
    string file_name = "";
    double precised_tss = 0;
    vector<int> hr_series;

    FileData(const string& file_name, double precised_tss, const vector<int>& hr_series)
        : file_name(file_name)
        , precised_tss(precised_tss)
        , hr_series(hr_series)
    {
    }

    FileData() {}
};

