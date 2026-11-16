#include "menu_policy.hpp"
#include "window_preferences.hpp"
#include "page_layout.hpp"
#include <stdexcept>
#include <iostream>
using namespace masiva::menu;
void require(bool value) {if(!value)throw std::runtime_error("menu policy failed");}
int main() {
    using namespace masiva;
    require(ui::opacity(-1)==50 && ui::opacity(500)==100);
    require(ui::scale(-1)==70 && ui::scale(500)==140);
    require(ui::alpha(100)==255 && ui::alpha(50)==128);
    for(int dpi:{96,120,144,192})for(int scale:{70,100,140})
        for(auto screen:{std::pair<int,int>{1280,720},{1920,1080},{2560,1440},{3840,2160}}){
            const int effective=ui::fitted_dpi(dpi,scale,screen.first,screen.second);
            require(1200*effective/96<=screen.first-32 && 720*effective/96<=screen.second-32);
            // Gallery hit coordinates remain aligned with scaled drawing.
            const int x=210*effective/96;
            require(std::abs(x*96/effective-210)<=2);
        }
    require(ui::fitted_dpi(96,70,3840,2160)<ui::fitted_dpi(96,140,3840,2160));
    for(int dpi:{48,67,96,120,144,192})for(auto dimensions:{std::pair<int,int>{1100,674},{720,650},{740,596},{460,470}}){
        const int width=1200*dpi/96;const auto child=page::fit_size(width,dpi,dimensions.first,dimensions.second);
        require(dimensions.first*child/96<=width*1168/1200);
        require(dimensions.second*child/96<=width*584/1200);
    }
    require(valid_key(default_key));require(valid_key(0x70));require(valid_key('K'));
    for(auto key:{0U,9U,13U,16U,17U,18U,27U,0xffU})require(!valid_key(key));
    require(toggle_allowed(true,false,false,false,false,default_key));
    require(toggle_allowed(false,true,false,false,false,default_key));
    require(!toggle_allowed(false,false,false,false,false,default_key));
    require(!toggle_allowed(true,false,true,false,false,default_key));
    require(!toggle_allowed(true,false,false,false,true,default_key));
    require(!toggle_allowed(false,true,false,true,false,'K'));
    require(toggle_allowed(false,true,false,true,false,default_key));
    for(std::size_t i=0;i<page_size;++i) {
        const auto r=card_rect(i);
        require(r.contains(r.x,r.y) && r.contains(r.x+r.w-1,r.y+r.h-1));
        require(!r.contains(r.x+r.w,r.y) && !r.contains(r.x,r.y+r.h));
        require(r.x>=184 && r.x+r.w<=824 && r.y>=158 && r.y+r.h<650);
        for(std::size_t j=0;j<page_size;++j)if(i!=j) {
            const auto other=card_rect(j);require(!other.contains(r.x+r.w/2,r.y+r.h/2));
        }
    }
    for(bool next:{false,true}) {
        const auto r=page_button_rect(next);
        require(r.y>=card_rect(5).y+card_rect(5).h+8);
        require(r.y+r.h<=674 && r.x>=184 && r.x+r.w<=824);
        for(int dpi:{67,96,120,144,192})require((r.y+r.h)*dpi/96<684*dpi/96);
    }
    require(pages(0)==1 && pages(6)==1 && pages(7)==2);
    require(clamp_page(100,7)==1 && card_index(1,0,7)==6);
    require(card_index(1,1,7)==7 && card_index(0,6,10)==10);
    for(std::size_t count=0;count<150;++count)
        for(std::size_t page=0;page<30;++page)
            for(std::size_t card=0;card<page_size;++card)require(card_index(page,card,count)<=count);
    std::cout<<"Menu scope, editing, key validation and page boundaries passed\n";
}
