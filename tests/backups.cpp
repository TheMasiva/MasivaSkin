#include "profile_backups.hpp"
#include <fstream>
#include <iostream>
#include <chrono>
#include <stdexcept>
namespace fs=std::filesystem;
std::string contents(const fs::path& p){std::ifstream f(p);return std::string(std::istreambuf_iterator<char>(f),{});}
int main(){
 const auto dir=fs::temp_directory_path()/("masiva-backups-"+std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));fs::create_directories(dir);
 try{const auto path=dir/"test.masiva";masiva::backup_profile(path);
 for(int i=0;i<50;++i){masiva::backup_profile(path);std::ofstream(path)<<i;}
 if(contents(path)!="49")throw std::runtime_error("current file changed");
 for(int i=1;i<=5;++i){auto p=path;p+=L".bak"+std::to_wstring(i);if(contents(p)!=std::to_string(49-i))throw std::runtime_error("backup order lost");}
 auto backup=path;backup+=L".bak1";const auto restored=contents(backup);masiva::backup_profile(path);std::ofstream(path)<<restored;
 if(contents(path)!="48" || contents(backup)!="49")throw std::runtime_error("restore did not preserve current version");
 auto ini=dir/"settings.ini";std::ofstream(ini)<<"keep";masiva::backup_profile(ini);if(fs::exists(dir/"settings.ini.bak1"))throw std::runtime_error("unrelated file backed up");
 fs::remove_all(dir);std::cout<<"Backup rotation, bounded history and restore passed\n";
 }catch(...){fs::remove_all(dir);throw;}
}
