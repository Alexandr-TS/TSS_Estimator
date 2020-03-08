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

#include "Utils.h"
#include "Research.h"

struct ResearchData {
    vector<FileData> files;
};

ResearchData ParseAndFilter(const string& directory_name) {
    filesystem::directory_iterator dir_iterator;
    try {
        dir_iterator = filesystem::directory_iterator(directory_name);
    }
    catch (const filesystem::filesystem_error&) {
        cout << "Error: No such directory: " << directory_name << ". ";
        cout << "Please check the directory path" << endl;
        return ResearchData();
    }

    ResearchData research_data;
    vector<future<FileData>> futures_data;

    for (const auto& file : dir_iterator) {
        futures_data.push_back(async([file] {
            auto file_path = file.path().string();
            ifstream fin(file_path.c_str());
            string row;

            vector<int> heart_rate_series;
            ActivityData activity_data;

            bool is_tss_counted = false;
            while (getline(fin, row)) {
                ProcessForHR(row, heart_rate_series);
                is_tss_counted |= ProcessForTSS(row, activity_data);
            }

            if (!is_tss_counted) {
                return FileData();
            }

            if (activity_data.total_elapsed_time > 2.0 * heart_rate_series.size()) {
                return FileData();
            }

            // Normalize tss because not all the rows can have heart rate
            return FileData(
                file_path,
                activity_data.tss / activity_data.total_elapsed_time * (double)heart_rate_series.size(),
                heart_rate_series
            );
            }));
    }

    for (auto& f : futures_data) {
        auto res = f.get();
        if (!res.file_name.empty()) {
            research_data.files.push_back(move(res));
        }
    }

    return research_data;
}

void Research(ResearchData& data) {
    int rest_hr = 50;
    int threshold_hr = 189;
    double alpha = 2.0;
    for (auto& file : data.files) {
        double acc = 0;
        for (auto hr : file.hr_series) {
            acc += pow(((double)hr - rest_hr) / ((double)threshold_hr - rest_hr), alpha);
        }

        auto min_el = *min_element(file.hr_series.begin(), file.hr_series.end());
        auto max_el = *max_element(file.hr_series.begin(), file.hr_series.end());
        double avg = accumulate(file.hr_series.begin(), file.hr_series.end(), 0.0) / file.hr_series.size();
        acc *= (100. / 3600.);
        cout << "total time: " << file.hr_series.size() << "   file: " << file.file_name << endl;
        cout << fixed << setprecision(2) << "counted: " << acc << endl;
        cout << fixed << setprecision(2) << "real: " << file.precised_tss << endl;
        cout << fixed << setprecision(2) << "min avg max: " << min_el << " " << avg << " " << max_el << endl;
        cout << endl;
    }
}

void LaunchResearch() {
    double start_time = (double)clock();
    auto data = ParseAndFilter("Resources/CSVs2017/");
    cout << data.files.size() << endl;
    Research(data);
    cout << "total time: " << ((double)clock() - start_time) / CLOCKS_PER_SEC << endl;
}
