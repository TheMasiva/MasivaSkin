#include "catalog.hpp"
#include <chrono>
#include <cstdio>
#include <iterator>
int main(){
 using namespace masiva::catalog;
 unsigned state=12345,checksum=0;
 auto start=std::chrono::steady_clock::now();
 for(int i=0;i<3000000;++i){
  state=state*1664525u+1013904223u;
  const auto& sample=finishes[state%std::size(finishes)];
  const auto* found=finish(sample.definition,sample.paint+(i%8==0?100000:0));
  if(found)checksum+=found->paint+1;
 }
 std::printf("%lld ms checksum %u\n",(long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-start).count(),checksum);
}
