#pragma once
#include <gdiplus.h>
#include <cmath>
namespace studio {
inline Gdiplus::Color color(COLORREF c){return {255,GetRValue(c),GetGValue(c),GetBValue(c)};}
inline void rounded(HDC dc,const RECT& r,COLORREF top,COLORREF bottom,COLORREF edge,float radius,float stroke=1) {
    if(r.right<=r.left || r.bottom<=r.top)return;
    Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    const float x=r.left+stroke/2,y=r.top+stroke/2,w=r.right-r.left-stroke,h=r.bottom-r.top-stroke;
    const float d=std::min(radius*2,std::min(w,h));Gdiplus::GraphicsPath path;
    path.AddArc(x,y,d,d,180,90);path.AddArc(x+w-d,y,d,d,270,90);
    path.AddArc(x+w-d,y+h-d,d,d,0,90);path.AddArc(x,y+h-d,d,d,90,90);path.CloseFigure();
    Gdiplus::LinearGradientBrush fill(Gdiplus::PointF(x,y),Gdiplus::PointF(x,y+h),color(top),color(bottom));
    g.FillPath(&fill,&path);if(stroke>0){Gdiplus::Pen pen(color(edge),stroke);g.DrawPath(&pen,&path);}
}
inline void outline(HDC dc,const RECT& r,COLORREF edge,float radius) {
    Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    const float x=r.left+.75f,y=r.top+.75f,w=r.right-r.left-1.5f,h=r.bottom-r.top-1.5f;
    const float d=std::min(radius*2,std::min(w,h));Gdiplus::GraphicsPath path;
    path.AddArc(x,y,d,d,180,90);path.AddArc(x+w-d,y,d,d,270,90);
    path.AddArc(x+w-d,y+h-d,d,d,0,90);path.AddArc(x,y+h-d,d,d,90,90);path.CloseFigure();
    Gdiplus::Pen pen(color(edge),1.5f);g.DrawPath(&pen,&path);
}
enum Icon {Gun,Knife,Glove,Agent,Star,Layers,Settings,Search,Filter,Sliders,Tag,Undo,Check,Chevron,Minus,Close};
inline void icon(HDC dc,Icon kind,int x,int y,int size,COLORREF ink) {
    Gdiplus::Graphics g(dc);g.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
    g.TranslateTransform(static_cast<float>(x),static_cast<float>(y));g.ScaleTransform(size/24.f,size/24.f);
    Gdiplus::Pen p(color(ink),1.6f);p.SetStartCap(Gdiplus::LineCapRound);p.SetEndCap(Gdiplus::LineCapRound);p.SetLineJoin(Gdiplus::LineJoinRound);
    auto line=[&](float a,float b,float c,float d){g.DrawLine(&p,a,b,c,d);};
    switch(kind) {
    case Gun: line(2,10,20,7);line(20,7,22,7);line(5,10,7,13);line(7,13,18,11);line(10,12,8,18);line(13,12,15,17);line(15,17,17,16);line(17,16,16,12);line(2,10,2,14);line(2,14,5,13);break;
    case Knife: line(5,19,19,5);line(19,5,17,13);line(17,13,11,16);line(8,13,12,17);line(5,19,7,21);line(7,21,11,17);break;
    case Glove: line(7,20,3,12);line(3,12,5,11);line(5,11,8,14);line(8,14,7,5);line(7,5,9,4);line(9,4,11,12);line(11,12,11,3);line(11,3,13,3);line(13,3,14,12);line(14,12,15,5);line(15,5,17,5);line(17,5,17,13);line(17,13,19,8);line(19,8,21,9);line(21,9,18,20);line(18,20,7,20);break;
    case Agent: g.DrawEllipse(&p,8.f,2.f,8.f,9.f);g.DrawArc(&p,3.f,11.f,18.f,13.f,180.f,180.f);line(3,17,3,21);line(3,21,21,21);line(21,21,21,17);break;
    case Star:{Gdiplus::PointF pts[10];for(int i=0;i<10;++i){float a=i*3.14159265f/5-1.5707963f,r=i%2?4.5f:10.f;pts[i]={12+r*std::cos(a),12+r*std::sin(a)};}g.DrawPolygon(&p,pts,10);break;}
    case Layers:line(3,8,12,3);line(12,3,21,8);line(21,8,12,13);line(12,13,3,8);line(3,12,12,17);line(12,17,21,12);line(3,16,12,21);line(12,21,21,16);break;
    case Settings:g.DrawEllipse(&p,5.f,5.f,14.f,14.f);g.DrawEllipse(&p,9.f,9.f,6.f,6.f);for(int i=0;i<8;++i){float a=i*3.14159265f/4;line(12+7*std::cos(a),12+7*std::sin(a),12+10*std::cos(a),12+10*std::sin(a));}break;
    case Search:g.DrawEllipse(&p,3.f,3.f,12.f,12.f);line(14,14,21,21);break;
    case Filter:line(3,5,21,5);line(21,5,14,13);line(14,13,14,20);line(14,20,10,18);line(10,18,10,13);line(10,13,3,5);break;
    case Sliders:line(3,6,21,6);line(3,12,21,12);line(3,18,21,18);g.DrawEllipse(&p,7.f,3.f,5.f,5.f);g.DrawEllipse(&p,14.f,9.f,5.f,5.f);g.DrawEllipse(&p,5.f,15.f,5.f,5.f);break;
    case Tag:line(3,3,12,3);line(12,3,22,13);line(22,13,13,22);line(13,22,3,12);line(3,12,3,3);g.DrawEllipse(&p,6.f,6.f,3.f,3.f);break;
    case Undo:g.DrawArc(&p,5.f,5.f,15.f,15.f,210.f,290.f);line(4,4,4,10);line(4,10,10,10);break;
    case Check:line(5,12,10,17);line(10,17,20,6);break;
    case Chevron:line(6,9,12,15);line(12,15,18,9);break;
    case Minus:line(5,12,19,12);break;
    case Close:line(6,6,18,18);line(18,6,6,18);break;
    }
}
}
