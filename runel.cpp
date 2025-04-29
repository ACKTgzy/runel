#include <iostream>
#include <filesystem>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <string>

using namespace std;

// 检查是否为 Ubuntu 系统
bool isUbuntu() {
    ifstream osRelease("/etc/os-release");
    string line;
    while (getline(osRelease, line)) {
        if (line.find("ID=ubuntu") != string::npos) {
            return true;
        }
    }
    return false;
}

bool has_access(const filesystem::path& p) {
    return access(p.c_str(), R_OK) == 0; // 检查读权限
}

std::vector<filesystem::path> findFiles(const filesystem::path& directory, const string& targetFileName) {
    vector<filesystem::path> foundPaths;
    try
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(directory)) {
            if (!has_access(entry.path())) { // 逐项权限检查
                continue;
            }
            if (entry.is_regular_file() && entry.path().filename() == targetFileName) {
                foundPaths.push_back(entry.path().lexically_normal());
            }
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        std::cerr << "Error1:" << e.what() << endl;
    }
    return foundPaths;
}

std::vector<filesystem::path> findFiles(const filesystem::path& directory, const string& targetFileName, int) {
    vector<filesystem::path> foundPaths;
    try
    {
        for (const auto& entry : filesystem::recursive_directory_iterator(directory)) {
            if (!has_access(entry.path())) { // 逐项权限检查
                continue;
            }
            if (entry.is_regular_file()) {
                string fullPath = entry.path().string();
                if (fullPath.find(targetFileName) != string::npos) {
                    foundPaths.push_back(entry.path().lexically_normal());
                }
            }
        }
    }
    catch (const filesystem::filesystem_error& e)
    {
        std::cerr << "Error2:" << e.what() << endl;
    }
    return foundPaths;
}

struct FilePath {
    string path;
    short fileMach = 0;
};

vector<filesystem::path> getRemovableMounts() {
    vector<filesystem::path> mounts;

    // 检查常见挂载目录（Linux/MacOS）
    for (const auto& entry : filesystem::directory_iterator("/media")) {
        mounts.push_back(entry.path());
    }
    for (const auto& entry : filesystem::directory_iterator("/mnt")) {
        mounts.push_back(entry.path());
    }

    // 通过系统挂载信息检测（Linux）
    ifstream mountsFile("/proc/mounts");
    string line;
    while (getline(mountsFile, line)) {
        if (line.find("usb") != string::npos || line.find("/dev/sd") != string::npos) {
            istringstream iss(line);
            string device, path;
            iss >> device >> path;
            mounts.emplace_back(path);
        }
    }

    return mounts;
}

FilePath runWhereFile(string targetFileName) {
    vector<filesystem::path> searchPaths;

    // 添加当前目录
    searchPaths.push_back(filesystem::current_path());

    // 添加可移动设备挂载点
    vector<filesystem::path> usbPaths = getRemovableMounts();
    searchPaths.insert(searchPaths.end(), usbPaths.begin(), usbPaths.end());

    // 在所有路径中搜索
    FilePath fileWhere;
    for (const auto& dir : searchPaths) {
        vector<filesystem::path> paths = findFiles(dir, targetFileName);
        if (!paths.empty()) {
            for (const auto& path : paths) {
                fileWhere.path += path.string();
                fileWhere.path += " ";
                fileWhere.fileMach++;
            }
        }
    }

    // 模糊匹配后备搜索
    if (fileWhere.fileMach == 0) {
        for (const auto& dir : searchPaths) {
            vector<filesystem::path> paths_fz = findFiles(dir, targetFileName, 0);
            if (!paths_fz.empty()) {
                for (const auto& path : paths_fz) {
                    fileWhere.path += path.string();
                    fileWhere.path += " ";
                    fileWhere.fileMach++;
                }
            }
        }
    }

    if (fileWhere.fileMach == 0) {
        cout << "Error: File not found in any mounted devices" << endl;
    }

    return fileWhere;
}

void handleCommand(const string& arg, const string& prefix = "") {
    FilePath fileWhere = runWhereFile(arg);
    if (fileWhere.fileMach == 0) exit(-1);

    if (fileWhere.fileMach > 1) {
        cout << "Ambiguous: " << fileWhere.path << endl;
        exit(fileWhere.fileMach);
    }

    string fullCmd = prefix + fileWhere.path;
    system(fullCmd.c_str());
}

int main(int argc, char* argv[]) {
    if (!isUbuntu()) {
        cerr << "This program is designed to run on Ubuntu only." << endl;
        return -1;
    }

    for (int i = 1; i < argc; ++i) {
        string cmd = argv[i];
        if (++i >= argc) {
            cout << "sdc:sudo run c exe" << endl;
            cout << "c:run c exe" << endl;
            cout << "sdc++:sudo run c++ exe" << endl;
            cout << "c++:run c++ exe" << endl;
            cout << "sdpy:sudo run python exe" << endl;
            cout << "py:run python exe" << endl;
            cout << "where:where file" << endl;
            cout << "seetxt:see txt" << endl;
            cout << "seeexe:see exe" << endl;
            cerr << "Missing argument" << endl;
            return -1;
        }

        string arg = argv[i];

        if (cmd == "where") {
            if (i < argc) {
                FilePath fileWhere = runWhereFile(arg);
                if (fileWhere.fileMach == 0) {
                    return -1;
                }
                if (fileWhere.fileMach > 1) {
                    cout << "what?" << fileWhere.path << endl;
                    return fileWhere.fileMach;
                }
                else {
                    cout << fileWhere.path << endl;
                }
            }
            else {
                cerr << "Missing filename after 'where'" << endl;
            }
        }
        else if (cmd == "sdc") handleCommand(arg, "sudo ");
        else if (cmd == "c") handleCommand(arg);
        else if (cmd == "sdc++") handleCommand(arg, "sudo ");
        else if (cmd == "c++") handleCommand(arg);
        else if (cmd == "sdpy") handleCommand(arg, "sudo python3 ");
        else if (cmd == "py") handleCommand(arg, "python3 ");
        else if (cmd == "seetxt") handleCommand(arg, "vi ");
        else if (cmd == "seeexe") handleCommand(arg, "hexdump -C ");
        else {
            cout << "runel -help" << endl;
        }
    }
    return 0;
}