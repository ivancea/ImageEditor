#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <SFML/Graphics.hpp>

struct BMPHeader{
    int8_t   fileType[2];
    uint32_t fileSize;
    uint16_t reserved1,
             reserved2;
    uint32_t imageDataStart,
             headerSize,
             width,
             height;
    uint16_t plainCount,
             bitsPerPixel;
    uint32_t compression,
             imageSize,
             horizontalResolution,
             verticalResolution,
             colorTableSize,
             importantColorCounter;
} __attribute__ ((__packed__));

class Image : public sf::Drawable{

    sf::Color **_m;
    int _x, _y;

    static void fill(sf::Color**& arr, int x, int y);
    static void destroy(sf::Color** arr, int x, int y);

    Image(sf::Color **m, int x, int y);

public:
    Image();
    Image(const Image& img);
    Image(Image&& img);

    Image& operator=(const Image& img);


    /// GETTERS AND SETTERS

    int getX() const { return _x;}
    int getY() const { return _y; }
    bool isValid() const { return _x>0 && _y>0 && _m!=nullptr; }
    sf::Color get(int x, int y) const {
        if(x<0 || x>=_x || y<0 || y>=_y)
            return sf::Color::Black;
        return _m[x][y];
    }
    void set(int x, int y, sf::Color color){
        if(x<0 || x>=_x || y<0 || y>=_y)
            return;
        _m[x][y] = color;
    }


    /// IMPORT AND EXPORT

    bool loadFromBMP(std::string fileName);
    bool saveToBMP(std::string fileName) const;

    bool loadFromPBM(std::string fileName);
    bool saveToPBM(std::string fileName) const;

    bool saveToAsciiArt(std::string fileName, int pixelsX = 4, int pixelsY = 8) const;


    /// IMAGE MANAGING

    void create(int x, int y, sf::Color p = sf::Color::Black);
    bool paste(const Image& img, int destOffsetX, int destOffsetY, int srcOffsetX=0, int srcOffsetY=0, int width=0, int height=0);
    Image copy(int offsetX=0, int offsetY=0, int width=-1, int height=-1) const;
    int compareTo(const Image& img) const;
    Image compareToMask(const Image& img, sf::Color equal = sf::Color::White, sf::Color different = sf::Color::Black) const;
    Image& scale(double scaleX, double scaleY);

    /// EFFECTS

    Image& charcoal(unsigned char tolerance=255);
    Image& grayscale();
    Image& blackAndWhite();
    Image& posterize(unsigned char level);
    Image& invert();
    Image& blur(const int size);
    Image& minimizeColors(bool r, bool g, bool b);
    Image& maximizeColors(bool r, bool g, bool b);
    Image& replaceColor(sf::Color toSearch, unsigned char tolerance, sf::Color newColor);
    Image& craze();
    Image& bloom(int range);
    Image& modifyLight(short increment);
    Image& cartoonize(unsigned char level=2, unsigned char tolerance=10);

    // MAY BE DELETED
    Image& cartoon(unsigned char level=2, unsigned char tolerance=10);

    /// HIERARCHY OVERRIDES

    virtual void draw(sf::RenderTarget& rt, sf::RenderStates rs) const;
};

#endif // IMAGE_H
