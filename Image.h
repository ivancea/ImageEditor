#ifndef IMAGE_H
#define IMAGE_H

#include <string>
#include <SFML/Graphics.hpp>

class Image : public sf::Drawable{
    sf::Color **_m;
    int _x, _y;

    static void fill(sf::Color**& arr, int x, int y);
    static void destroy(sf::Color** arr, int x, int y);

    Image(sf::Color **m, int x, int y);

public:
    Image();
    Image(const Image& img);


    /// GETTERS AND SETTERS

    int getX() const { return _x; }
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

    bool loadFromPBM(std::string archivo);
    bool saveToPBM(std::string archivo) const;


    /// IMAGE MANAGING

    void create(int x, int y, sf::Color p = sf::Color::Black);
    bool paste(const Image& img, int destOffsetX, int destOffsetY, int srcOffsetX=0, int srcOffsetY=0, int width=0, int height=0);
    Image copy(int offsetX=0, int offsetY=0, int width=0, int height=0) const;

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


    /// HIERARCHY OVERRIDES

    virtual void draw(sf::RenderTarget& rt, sf::RenderStates rs) const;
};

#endif // IMAGE_H
