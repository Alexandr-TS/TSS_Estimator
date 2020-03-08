#include "Utils.h"

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

optional<double> ProcessForElapsedTime(const string& row) {
    if (!CheckRowIsData(row)) {
        return {};
    }
    if (row.find(",session,") == string::npos) {
        return {};
    }
    auto resp = GetValueAfterKeyInCSVRow(row, ",total_elapsed_time");
    if (!resp) {
        return {};
    }
    istringstream is(resp.value());
    double val;
    is >> val;
    return val;
}

