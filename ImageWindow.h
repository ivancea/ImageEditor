#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <map>
#include <SFML/Graphics.hpp>

#include "MutexedImage.h"

class ImageWindow{
    sf::RenderWindow _window;
    sf::Vector2i _mouse;

    std::string _imageName;

    MutexedImage* prepareWindow(std::map<std::string,MutexedImage*>& images);

public:
    ImageWindow(){}
    ImageWindow(std::string imageName);
    ImageWindow(const ImageWindow&) = delete;
    ~ImageWindow(){
        _window.setActive(false);
    }

    std::string getImageName() const { return _imageName; }
    void setImageName(std::string imageName) {
        _imageName = imageName;
    }

    bool loop(std::map<std::string,MutexedImage*>& images);
};

#endif // IMAGEWINDOW_H
