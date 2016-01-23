#include "Image.h"

#include <fstream>

Image::Image():_m(nullptr),_x(0),_y(0){}

Image::Image(sf::Color **m, int x, int y):_m(m),_x(x),_y(y){}

void Image::fill(sf::Color**& arr, int x, int y){
    if(x==0 || y==0){
        arr=nullptr;
        return;
    }
    arr = new sf::Color*[x];
    for(int i=0; i<x; i++)
        arr[i] = new sf::Color[y];
}

void Image::destroy(sf::Color** arr, int x, int y){
    if(arr==nullptr) return;
    if(y>0)
        for(int i=0; i<x; i++)
            delete[] arr[i];
    delete[] arr;
}

void Image::create(int x, int y, sf::Color p){
    if(x<1||y<1) return;
    if(_x>0 && _y>0)
        destroy(_m, _x, _y);
    Image::fill(_m, x, y);
    for(int i=0; i<x; i++)
        for(int j=0; j<y; j++)
            _m[i][j] = p;
    _x=x;
    _y=y;
}

bool Image::loadFromPBM(std::string archivo){
    int x=0, y=0;
    int tipo;
    std::ifstream f(archivo,std::ios::binary);
    if(!f) return false;

    std::string t;
    getline(f,t,(char)0x0A);
    if(t.size()!=2 || (t[0]!='P' && t[0]!='p') || t[1]<'1' || t[1]>'6')
        return false;
    tipo = t[1]-'0';
    getline(f,t,(char)0x0A);
    x = stoi(t);
    getline(f,t,(char)0x0A);
    y = stoi(t);
    getline(f,t,(char)0x0A); //Depth
    if(y==0 || x==0)
        return false;

    fill(_m,x,y);
    for(int j=0;j<y;j++)
        for(int i=0;i<x;i++){
            f.read((char*)&_m[i][j].r, 1);
            f.read((char*)&_m[i][j].g, 1);
            f.read((char*)&_m[i][j].b, 1);
        }
    _x=x;
    _y=y;
    return true;
}

bool Image::saveToPBM(std::string archivo) const {
    if(!isValid()) return false;
    std::ofstream f(archivo,std::ios::trunc|std::ios::binary);
    if(!f) return false;
    char del = 0x0A;
    f << "P6";
    f.write(&del,1);
    f << _x;
    f.write(&del,1);
    f << _y;
    f.write(&del,1);
    f << "255";
    f.write(&del,1);
    for(int j=0;j<_y;j++)
        for(int i=0;i<_x;i++){
            f.write((char*)&_m[i][j].r,1);
            f.write((char*)&_m[i][j].g,1);
            f.write((char*)&_m[i][j].b,1);
        }
    return true;
}

Image& Image::charcoal(unsigned char tolerance){
    sf::Color **t;
    fill(t, _x, _y);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            for(int n=-1; n<=1; n++){
                for(int m=-1; m<=1; m++){
                    if((n!=0 || m!=0) && i+n>=0 && i+n<_x && j+m>=0 && j+m<_y)
                        if((int)_m[i+n][j+m].r-(int)_m[i][j].r>tolerance ||
                           (int)_m[i+n][j+m].g-(int)_m[i][j].g>tolerance ||
                           (int)_m[i+n][j+m].b-(int)_m[i][j].b>tolerance)
                            break;
                    if(n==1 && m==1){
                        t[i][j] = sf::Color(255,255,255);
                    }
                }
            }
    destroy(_m,_x,_y);
    _m = t;
    return *this;
}

Image& Image::Image::grayscale(){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            int t = (_m[i][j].r+_m[i][j].g+_m[i][j].b)/3;
            _m[i][j] = sf::Color(t,t,t);
        }
    return *this;
}
Image& Image::blackAndWhite(){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            double dt = _m[i][j].r+_m[i][j].g+_m[i][j].b;
            _m[i][j] = (dt/3>=127.5?sf::Color::White:sf::Color::Black);
        }
    return *this;
}

Image& Image::posterize(unsigned char level){
    if(level<2) level = 2;
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            double value = double(_m[i][j].r)/255.0;
            value *=level-1;
            value  = round(value);
            value /=level-1;
            _m[i][j].r = value*255;
            value  = double(_m[i][j].g)/255.0;
            value *=level-1;
            value  = round(value);
            value /=level-1;
            _m[i][j].g = value*255;
            value  = double(_m[i][j].b)/255.0;
            value *=level-1;
            value  = round(value);
            value /=level-1;
            _m[i][j].b = value*255;
        }
    return *this;
}

Image& Image::invert(){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            _m[i][j].r = 255-_m[i][j].r;
            _m[i][j].g = 255-_m[i][j].g;
            _m[i][j].b = 255-_m[i][j].b;
        }
    return *this;
}

Image& Image::blur(const int size){
    sf::Color **t;
    fill(t,_x,_y);
    int s = (1+size*2);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            int r,g,b,c;
            r=g=b=c=0;
            for(int n=0; n<=s*s; n++){
                if(i+n%s-size>=0 && i+n%s-size<_x && j+n/s-size>=0 && j+n/s-size<_y){
                    ++c;
                    r+=_m[i+n%s-size][j+n/s-size].r;
                    g+=_m[i+n%s-size][j+n/s-size].g;
                    b+=_m[i+n%s-size][j+n/s-size].b;
                }
            }
            if(c!=0)
                t[i][j] = sf::Color(r/c,g/c,b/c);
        }

    destroy(_m,_x,_y);
    _m = t;
    return *this;
}

Image& Image::minimizeColors(bool r, bool g, bool b){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            if(r) _m[i][j].r=0;
            if(g) _m[i][j].g=0;
            if(b) _m[i][j].b=0;
        }
    return *this;
}
Image& Image::maximizeColors(bool r, bool g, bool b){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            if(r) _m[i][j].r=255;
            if(g) _m[i][j].g=255;
            if(b) _m[i][j].b=255;
        }
    return *this;
}

Image& Image::replaceColor(sf::Color toSearch, unsigned char tolerance, sf::Color newColor){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            if(abs((int)_m[i][j].r+(int)_m[i][j].g+(int)_m[i][j].b
                -(int)toSearch.r-(int)toSearch.g-(int)toSearch.b)<=tolerance)
                _m[i][j] = newColor;
        }
    return *this;
}

Image& Image::craze(){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            bool r,g,b;
            r=g=b=false;
            if(_m[i][j].r<128) r=true;
            if(_m[i][j].g<128) g=true;
            if(_m[i][j].b<128) b=true;
            if(r){
                _m[i][j].g/=2;
                _m[i][j].b/=2;
            }else{
                _m[i][j].r+=(255-_m[i][j].r)/2;
            }
            if(g){
                _m[i][j].r/=2;
                _m[i][j].b/=2;
            }else{
                _m[i][j].g+=(255-_m[i][j].g)/2;
            }
            if(b){
                _m[i][j].g/=2;
                _m[i][j].r/=2;
            }else{
                _m[i][j].b+=(255-_m[i][j].b)/2;
            }
        }
    return *this;
}

Image& Image::bloom(int range){
    sf::Color** t;
    fill(t,_x,_y);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            int r=0, g=0, b=0;
            int rx=0, ry=0,
                gx=0, gy=0,
                bx=0, by=0;
            for(int n=(i-range<0?0:-range); n<=range && i+n<_x; n++)
                for(int m=(j-range<0?0:-range); m<=range && j+m<_y && n!=m; m++){
                    if(_m[i+n][j+m].r>r){
                        r = _m[i+n][j+m].r;
                        rx = n;
                        ry = m;
                    }
                    if(_m[i+n][j+m].g>g){
                        g = _m[i+n][j+m].g;
                        gx = n;
                        gy = m;
                    }
                    if(_m[i+n][j+m].b>b){
                        b = _m[i+n][j+m].b;
                        bx = n;
                        by = m;
                    }
                }
            float rd = sqrt(rx*rx+ry*ry),
                  gd = sqrt(gx*gx+gy*gy),
                  bd = sqrt(bx*bx+by*by);
            t[i][j] = _m[i][j];
            if(r-(int)t[i][j].r > 0){
                t[i][j].r = (r*5+t[i][j].r*rd)/(rd+5);
            }
            if(g-(int)t[i][j].g > 0){
                t[i][j].g = (g*5+t[i][j].g*gd)/(gd+5);
            }
            if(b-(int)t[i][j].b > 0){
                t[i][j].b = (b*5+t[i][j].b*bd)/(bd+5);
            }
        }
    destroy(_m,_x,_y);
    _m=t;
    return *this;
}

void Image::draw(sf::RenderTarget& rt, sf::RenderStates rs)const{
    sf::Image img;
    img.create(_x, _y);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            img.setPixel(i,j, _m[i][j]);
    sf::Texture tx;
    tx.loadFromImage(img);
    rt.draw(sf::Sprite(tx));
}
