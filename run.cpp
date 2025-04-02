#include <iostream>
#include <filesystem>
#include <vector>
using namespace std;

std::vector<filesystem::path> findFiles(const filesystem::path& directory, const string& targetFileName){
    vector<filesystem::path> foundPaths;
    try
    {
        for(const auto& entry : filesystem::recursive_directory_iterator(directory)){
            if(entry.is_regular_file() && entry.path().filename() == targetFileName){
                foundPaths.push_back(entry.path().lexically_normal());
            }
        }
    }
    catch(const filesystem::filesystem_error& e)
    {
        std::cerr <<"Error:"<< e.what() << endl;
    }
    return foundPaths;
} 

struct FilePath{
    string path;
    short fileMach=0;
};

FilePath runWhereFile(string targetFileName) {
   filesystem::path startDirectory = filesystem::current_path();
   vector<filesystem::path> paths =findFiles(startDirectory,targetFileName);
   static FilePath fileWhere;
   if(paths.empty()){
        cout<<"Error: no file"<<endl;
   }
   else{
        for (const auto& path : paths)
        {
            fileWhere.path += path.string();
            fileWhere.fileMach++;
        }
   }
   return fileWhere;
}

int main(int argc, char* argv[]) {
    cout<<"runel"<<endl;
    string ls;
    for (int n = 1; n < argc; ++n) {  
        ls = argv[n];
        if (ls == "where") {
            if (++n < argc) {        
                ls = argv[n];
                FilePath fileWhere = runWhereFile(ls);
                if(fileWhere.fileMach > 1){
                    cout<<"what?"<<fileWhere.path<<endl;
                }
                else{
                    cout<<fileWhere.path<<endl;
                }
            } else {
                cerr << "Missing filename after 'where'" << endl;
            }
        }
        if(ls == "c" || ls == "c++"){
            if (++n < argc) {
                ls = argv[n];
                FilePath fileWhere = runWhereFile(ls);
                if(fileWhere.fileMach > 1){
                    cout<<"what?"<<fileWhere.path<<endl;
                }
                else{
                    string sudo = "sudo " + fileWhere.path;
                    std::system(sudo.c_str());
                }
            }
        }
        if(ls == "seeexe"){
            if (++n < argc) {
                ls = argv[n];
                FilePath fileWhere = runWhereFile(ls);
                if(fileWhere.fileMach > 1){
                    cout<<"what?"<<fileWhere.path<<endl;
                }
                else{
                    string sudo = "hexdump -C " + fileWhere.path;
                    std::system(sudo.c_str());
                }
            }
        }
        if(ls == "seetxt"){
            if (++n < argc) {
                ls = argv[n];
                FilePath fileWhere = runWhereFile(ls);
                if(fileWhere.fileMach > 1){
                    cout<<"what?"<<fileWhere.path<<endl;
                }
                else{
                    string sudo = "vi " + fileWhere.path;
                    std::system(sudo.c_str());
                }
            }
        }
    }
    return 0;
}