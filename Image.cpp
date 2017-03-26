#include "Image.h"

#include <iostream>
#include <fstream>
#include <cmath>

Image::Image():_m(nullptr),_x(0),_y(0){}

Image::Image(sf::Color **m, int x, int y):_m(m),_x(x),_y(y){}

Image::Image(const Image& img){
    _x = img.getX();
    _y = img.getY();
    if(_x>0 && _y>0){
        fill(_m, _x,_y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                _m[i][j] = img.get(i,j);
    }
}

Image::Image(Image&& img){
    _m = img._m;
    _x = img._x;
    _y = img._y;
    img._m = nullptr;
    img._x = 0;
    img._y = 0;
}

Image& Image::operator=(const Image& img){
    destroy(_m, _x,_y);
    _x = img.getX();
    _y = img.getY();
    if(_x>0 && _y>0){
        fill(_m, _x,_y);
        for(int i=0; i<_x; i++)
            for(int j=0; j<_y; j++)
                _m[i][j] = img.get(i,j);
    }
}

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

// TODO: Fix bugs (srcOffset can overflow on Image::get)
bool Image::paste(const Image& img, int destOffsetX, int destOffsetY, int srcOffsetX, int srcOffsetY, int width, int height){
    if(!this->isValid() || !img.isValid()
    || destOffsetX>=_x || destOffsetY>=_y
    || srcOffsetX>=img.getX() || srcOffsetY>=img.getY())
        return false;
    if(width<=0 || width+srcOffsetX>=img.getX() || width+destOffsetX>=_x)
        width = std::min(img.getX()-srcOffsetX, _x-destOffsetX);
    if(height<=0 || height+srcOffsetY>=img.getY() || height+destOffsetY>=_y)
        height = std::min(img.getY()-srcOffsetY, _y-destOffsetY);

    if(destOffsetX+width<=0 || destOffsetY+height<=0)
        return false;

    if(destOffsetX<0){
        width += destOffsetX;
        destOffsetX = 0;
    }
    if(destOffsetY<0){
        height += destOffsetY;
        destOffsetY = 0;
    }

    for(int i=0; i<width; i++)
        for(int j=0; j<height; j++)
            _m[destOffsetX+i][destOffsetY+j] = img.get(srcOffsetX+i, srcOffsetY+j);
    return true;
}

Image Image::copy(int offsetX, int offsetY, int width, int height) const{
    Image ret;
    if(width==-1)
        width = _x;
    if(height==-1)
        height = _y;
    if(width<=0 || height<=0
    || offsetX>=_x || offsetY>=_y
    || offsetX+width<0 || offsetY+height<0)
        return ret;
    if(offsetX<0){
        width += offsetX;
        offsetX = 0;
    }
    if(offsetY<0){
        height += offsetY;
        offsetY = 0;
    }
    if(offsetX+width>=_x)
        width = _x-offsetX;
    if(offsetY+height>=_x)
        height = _x-offsetY;
    ret.create(width, height);
    for(int i=0; i<width; i++)
        for(int j=0; j<height; j++)
            ret.set(i,j, _m[offsetX+i][offsetY+j]);
    return ret;
}

int Image::compareTo(const Image& img) const{
    if(!isValid() || !img.isValid()
    || _x != img.getX() || _y != img.getY())
        return -1;
    int fails = 0;
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            if(_m[i][j] != img.get(i,j))
                ++fails;
    return fails;
}

Image Image::compareToMask(const Image& img, sf::Color equal, sf::Color different) const{
    Image t;
    if(!isValid() || !img.isValid()
    || _x != img.getX() || _y != img.getY())
        return t;
    t.create(_x,_y, equal);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            if(_m[i][j] != img.get(i,j))
                t.set(i,j, different);
    return t;
}

/*Image& Image::scaleBasic(double scaleX, double scaleY){
    if(scaleX <= 0 || scaleY <= 0)
        return *this;
    sf::Color **t;
    int x = _x*scaleX,
        y = _y*scaleY;
    fill(t, x,y);

    for(int i=0; i<x; i++)
        for(int j=0; j<y; j++){
            t[i][j] = _m[int(i/scaleX)][int(j/scaleY)];
        }
    destroy(_m, _x,_y);
    _m = t;
    _x = x;
    _y = y;
    return *this;
}*/

Image& Image::scale(double scaleX, double scaleY){
    if(scaleX <= 0 || scaleY <= 0)
        return *this;
    sf::Color **t;
    int x = _x*scaleX,
        y = _y*scaleY;
    fill(t, x,y);

    for(int i=0; i<x; i++)
        for(int j=0; j<y; j++){
            sf::Vector3f newColor;
            int total=0;
            for(int n=floor(i/scaleX); n<=ceil(i/scaleX) && n<_x; n++)
                for(int m=floor(j/scaleY); m<=ceil(j/scaleY) && m<_y; m++){
                    sf::Vector2f p(n-i/scaleX, m-j/scaleY);
                    sf::Vector3f c;
                    c.x = _m[n][m].r*(1-abs(n-i/scaleX))*(1-abs(m-j/scaleY));
                    c.y = _m[n][m].g*(1-abs(n-i/scaleX))*(1-abs(m-j/scaleY));
                    c.z = _m[n][m].b*(1-abs(n-i/scaleX))*(1-abs(m-j/scaleY));
                    newColor += c;
                    total++;
                }
            if(total==0)
                total = 1;
            t[i][j] = sf::Color(newColor.x/total, newColor.y/total, newColor.z/total);
        }
    destroy(_m, _x,_y);
    _m = t;
    _x = x;
    _y = y;
    return *this;
}

/// BUILD WITH "-mno-ms-bitfields" in GCC
bool Image::loadFromBMP(std::string fileName){
    std::ifstream f(fileName,std::ios::binary);
    if(!f) return false;
    BMPHeader header;
    f.read((char*)&header, sizeof(header));
    /*std::cout << "FileType: " << header.fileType[0] << header.fileType[1] << "\n"
         << "FileSize: " << header.fileSize << "\n"
         << "Reserved1: " << header.reserved1 << "\n"
         << "Reserved2: " << header.reserved2 << "\n"
         << "ImageDataStart: " << header.imageDataStart << "\n"
         << "HeaderSize: " << header.headerSize << "\n"
         << "Width: " << header.width << "\n"
         << "Height: " << header.height << "\n"
         << "PlainCount: " << header.plainCount << "\n"
         << "BitsPerPixel: " << header.bitsPerPixel << "\n"
         << "Compression: " << header.compression << "\n"
         << "ImageSize: " << header.imageSize << "\n"
         << "HorizontalResolution: " << header.horizontalResolution << "\n"
         << "VerticalResolution: " << header.verticalResolution << "\n"
         << "ColorTableSize: " << header.colorTableSize << "\n"
         << "ImportantColorCounter: " << header.importantColorCounter << "\n" << std::endl;*/
    if(header.width<=0 ||header.height<=0 || header.compression!=0
    || header.fileType[0]!='B' || header.fileType[1]!='M'
    || header.bitsPerPixel!=24)
        return false;
    if(isValid())
        destroy(_m, _x,_y);
    _x = header.width;
    _y = header.height;

    int padding = ((_x*3)%4==0? (_x*3)%4 : 4-(_x*3)%4 );
    char *buff = new char[_x*_y*3+padding*_y];
    f.read(buff, _x*_y*3+padding*_y);
    fill(_m, _x,_y);

    char *t = buff;
    for(int j=_y-1; j>=0; j--){
        for(int i=0; i<_x; i++){
            _m[i][j].r = t[2];
            _m[i][j].g = t[1];
            _m[i][j].b = t[0];
            t += 3;
        }
        t += padding;
    }
    delete[] buff;
    return true;
}

/// BUILD WITH "-mno-ms-bitfields" in GCC
bool Image::saveToBMP(std::string fileName) const{
    if(!isValid()) return false;
    std::ofstream f(fileName, std::ios::trunc|std::ios::binary);
    if(!f) return false;
    BMPHeader header;
    header.fileType[0] = 'B';
    header.fileType[1] = 'M';
    header.fileSize = _x*_y*3+54;
    header.reserved1 = header.reserved2 = 0;
    header.imageDataStart = 54;
    header.headerSize = 40;
    header.width = _x;
    header.height = _y;
    header.plainCount = 1;
    header.bitsPerPixel = 24;
    header.compression = 0;
    header.imageSize = _x*_y*3;
    header.horizontalResolution = 2834; // Maybe
    header.verticalResolution = 2834; // Maybe
    header.colorTableSize = 0;
    header.importantColorCounter = 0;
    f.write((char*)&header, sizeof(header));
    char padding[((_x*3)%4==0? (_x*3)%4 : 4-(_x*3)%4 )];
    for(int j=_y-1; j>=0; j--){
        for(int i=0; i<_x; i++)
            f << _m[i][j].b << _m[i][j].g << _m[i][j].r;
        f.write(padding, sizeof(padding));
    }
    return true;
}

bool Image::loadFromPBM(std::string fileName){
    int x=0, y=0;
    int tipo;
    std::ifstream f(fileName,std::ios::binary);
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

bool Image::saveToPBM(std::string fileName) const {
    if(!isValid()) return false;
    std::ofstream f(fileName,std::ios::trunc|std::ios::binary);
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

bool Image::saveToAsciiArt(std::string fileName, int pixelsX, int pixelsY) const {
    if(!isValid()) return false;
    std::ofstream f(fileName,std::ios::trunc);
    if(!f) return false;
    static const std::string chars = " -+OHM";
    for(int j=0; j<_y/pixelsY; j++){
        for(int i=0; i<_x/pixelsX; i++){
            int t = 0;
            int count = 0;
            for(int n=0; n<pixelsX && i*pixelsX+n<_x; n++){
                for(int m=0; m<pixelsY && j*pixelsY+m<_y; m++){
                    count++;
                    t += _m[i*pixelsX+n][j*pixelsY+m].r + _m[i*pixelsX+n][j*pixelsY+m].g + _m[i*pixelsX+n][j*pixelsY+m].b;
                }
            }
            t /= count*3;
            f << chars[(int)(t/255.0 * chars.size())];
        }
        if(j!=_y/pixelsY-1)
            f << '\n';
    }
    return true;
}

Image& Image::charcoal(unsigned char tolerance){
    sf::Color **t;
    fill(t, _x, _y);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            for(int n=-1; n<=1; n++){
                bool mustEnd = false;
                for(int m=-1; m<=1 && !mustEnd; m++){
                    if((n!=0 || m!=0) && i+n>=0 && i+n<_x && j+m>=0 && j+m<_y)
                        if((int)_m[i+n][j+m].r-(int)_m[i][j].r>tolerance ||
                           (int)_m[i+n][j+m].g-(int)_m[i][j].g>tolerance ||
                           (int)_m[i+n][j+m].b-(int)_m[i][j].b>tolerance){
                            mustEnd = true;
                            break;
                        }
                    if(n==1 && m==1){
                        t[i][j] = sf::Color(255,255,255);
                    }
                }
                if(mustEnd)
                    break;
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
                -(int)toSearch.r-(int)toSearch.g-(int)toSearch.b)<=tolerance*3)
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

Image& Image::modifyLight(short increment){
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++){
            if(_m[i][j].r+increment>=255)
                _m[i][j].r=255;
            else if(_m[i][j].r+increment<=0)
                _m[i][j].r=0;
            else _m[i][j].r+=increment;

            if(_m[i][j].g+increment>=255)
                _m[i][j].g=255;
            else if(_m[i][j].g+increment<=0)
                _m[i][j].g=0;
            else _m[i][j].g+=increment;

            if(_m[i][j].b+increment>=255)
                _m[i][j].b=255;
            else if(_m[i][j].b+increment<=0)
                _m[i][j].b=0;
            else _m[i][j].b+=increment;
        }
    return *this;
}

Image& Image:: cartoonize(unsigned char level, unsigned char tolerance){
    sf::Color **post, **t;
    fill(post,_x,_y);
    posterize(level);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            post[i][j] = _m[i][j];
    charcoal(tolerance);
    t = _m;
    _m = post;
    post = t;
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            if(post[i][j]==sf::Color::Black)
                _m[i][j] = sf::Color::Black;
    destroy(post,_x,_y);
    return *this;
}

// MAY BE DELETED
Image& Image:: cartoon(unsigned char level, unsigned char tolerance){
    sf::Color **post, **t;
    fill(post,_x,_y);
    posterize(level);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            post[i][j] = _m[i][j];
    charcoal(tolerance);
    t = _m;
    _m = post;
    post = t;
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            if(post[i][j]!=sf::Color::Black)
                _m[i][j] = sf::Color::Black;
    destroy(post,_x,_y);
    return *this;
}

void Image::draw(sf::RenderTarget& rt, sf::RenderStates rs)const{
    if(_x == 0 || _y == 0)
        return;
    sf::Image img;
    img.create(_x, _y);
    for(int i=0; i<_x; i++)
        for(int j=0; j<_y; j++)
            img.setPixel(i,j, _m[i][j]);
    sf::Texture tx;
    tx.loadFromImage(img);
    rt.draw(sf::Sprite(tx));
}
