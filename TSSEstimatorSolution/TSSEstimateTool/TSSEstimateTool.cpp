#include "Utils.h"

void ProcessCSVs(const string& directory_name, int rest_hr, int threshold_hr) {
    filesystem::directory_iterator dir_iterator;
    try {
        dir_iterator = filesystem::directory_iterator(directory_name);
    }
    catch (const filesystem::filesystem_error&) {
        cout << "Error: No such directory: " << directory_name << ". ";
        cout << "Please check the directory path" << endl;
        return;
    }

    for (const auto& file : dir_iterator) {
        auto file_path = file.path().string();
        ifstream fin(file_path.c_str());
    
        string row;
        vector<int> heart_rate_series;
        double total_elapsed_time = -1.;
        while (getline(fin, row)) {
            ProcessForHR(row, heart_rate_series);
            auto time_info = ProcessForElapsedTime(row);
            if (time_info) {
                total_elapsed_time = time_info.value();
            }
        }

        if (total_elapsed_time == -1.) {
            cout << "Error in " << file_path << ":  no total elapsed time in data file" << endl;
            continue;
        }

        double tss = 0;
        for (auto hr : heart_rate_series) {
            tss += 100. / 3600. * pow(((double)hr - rest_hr) / ((double)threshold_hr - rest_hr), 2.);
        }
        tss *= total_elapsed_time / heart_rate_series.size();
        cout << file_path << ". Estimated TSS = " << fixed << setprecision(1) << tss << endl;
    }
}

int main() {
	system("ClearCSVs.bat");
	system("ActivitiesToCSVs.bat");
	ProcessCSVs("CSVs", 49, 189);
}
