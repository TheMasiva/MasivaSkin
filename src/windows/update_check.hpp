#pragma once
#include <winhttp.h>
#include <thread>
#include <atomic>
#include <memory>
#include "../update_metadata.hpp"
namespace masiva {
struct UpdateResult {std::atomic<bool> done{false};DumpMetadata metadata;std::string error;};
struct InternetHandle {HINTERNET value{};explicit InternetHandle(HINTERNET h):value(h){if(!h)throw std::runtime_error("Update connection unavailable");}~InternetHandle(){WinHttpCloseHandle(value);}InternetHandle(const InternetHandle&)=delete;};
inline std::shared_ptr<UpdateResult> check_upstream_dump(){
 auto result=std::make_shared<UpdateResult>();
 std::thread([result]{
  try{
   InternetHandle session(WinHttpOpen(L"MasivaSkin/0.9.2",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS,0));
   WinHttpSetTimeouts(session.value,3000,3000,3000,3000);
   InternetHandle connection(WinHttpConnect(session.value,L"raw.githubusercontent.com",INTERNET_DEFAULT_HTTPS_PORT,0));
   InternetHandle request(WinHttpOpenRequest(connection.value,L"GET",L"/a2x/cs2-dumper/main/output/info.json",nullptr,WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE));
   DWORD redirect=WINHTTP_OPTION_REDIRECT_POLICY_NEVER;WinHttpSetOption(request.value,WINHTTP_OPTION_REDIRECT_POLICY,&redirect,sizeof(redirect));
   if(!WinHttpSendRequest(request.value,WINHTTP_NO_ADDITIONAL_HEADERS,0,WINHTTP_NO_REQUEST_DATA,0,0,0)||!WinHttpReceiveResponse(request.value,nullptr))throw std::runtime_error("Update source unreachable");
   DWORD status{},size=sizeof(status);if(!WinHttpQueryHeaders(request.value,WINHTTP_QUERY_STATUS_CODE|WINHTTP_QUERY_FLAG_NUMBER,WINHTTP_HEADER_NAME_BY_INDEX,&status,&size,WINHTTP_NO_HEADER_INDEX)||status!=200)throw std::runtime_error("Update source returned an error");
   std::string body;char chunk[1024];DWORD n{};
   while(WinHttpReadData(request.value,chunk,sizeof(chunk),&n)&&n){body.append(chunk,n);if(body.size()>4096)throw std::runtime_error("Update metadata too large");}
   result->metadata=parse_dump_metadata(body);
  }catch(const std::exception& e){result->error=e.what();}
  result->done.store(true,std::memory_order_release);
 }).detach();return result;
}
}
