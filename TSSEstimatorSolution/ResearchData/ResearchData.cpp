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

optional<string> GetValueAfterKeyInCSVRow(const string& row, const string& key) {
    size_t pos = row.find(key);
    if (pos == string::npos) {
        return {};
    }
    auto quote_1_it = find(row.begin() + pos, row.end(), '\"');
    auto quote_2_it = find(next(quote_1_it), row.end(), '\"');
    assert(quote_2_it != row.end());
    assert(*prev(quote_1_it) == ',');
    assert(quote_1_it - row.begin() == pos + key.size() + 1);
    return string(next(quote_1_it), quote_2_it);
}

bool CheckRowIsData(const string& row) {
    const string wanted_prefix = "Data";
    return (row.size() >= wanted_prefix.size() &&
        equal(row.begin(), row.begin() + wanted_prefix.size(), wanted_prefix.begin()));
}

bool ProcessForHR(const string& row, vector<int>& heart_rate_series) {
    if (!CheckRowIsData(row)) {
        return false;
    }
    auto hr = GetValueAfterKeyInCSVRow(row, ",heart_rate");
    if (!hr) {
        return false;
    }
    heart_rate_series.emplace_back(stoi(hr.value(), nullptr, 10));
    return true;
}

struct ActivityData {
    double total_elapsed_time = -1.0;
    double tss = -1.0;
};

bool ProcessForTSS(const string& row, ActivityData& activity_data) {
    if (!CheckRowIsData(row)) {
        return false;
    }
    if (row.find(",session,") == string::npos) {
        return false;
    }
    auto total_elapsed_time = GetValueAfterKeyInCSVRow(row, ",total_elapsed_time");
    auto tss = GetValueAfterKeyInCSVRow(row, ",training_stress_score");
    if (!total_elapsed_time || !tss) {
        return false;
    }
    istringstream is1(total_elapsed_time.value());
    istringstream is2(tss.value());
    is1 >> activity_data.total_elapsed_time;
    is2 >> activity_data.tss;
    return true;
}

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
            acc += pow((double)(hr - rest_hr) / (threshold_hr - rest_hr), alpha);
        }
        /*
        for (auto& fn: data.files) {
            cout << fixed << setprecision(2) << fn.precised_tss / (double)fn.hr_series.size() * 60 * 60 << "tss/h ";
            cout << fn.precised_tss << " " << fn.hr_series.size() << " ";
            cout << fn.file_name << endl;
        }
        */

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

int main() {
    double start_time = (double)clock();
    auto data = ParseAndFilter("Resources/CSVs2017/");
    cout << data.files.size() << endl;
    Research(data);
    cout << "total time: " << ((double)clock() - start_time) / CLOCKS_PER_SEC << endl;
}