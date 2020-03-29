#ifndef IMAGEWINDOW_H
#define IMAGEWINDOW_H

#include <map>
#include <SFML/Graphics.hpp>

#include "MutexedImage.hpp"

class ImageWindow{
    sf::RenderWindow _window;
    sf::Vector2i _mouse;

    bool _mustPrepare;
    std::string _imageName;

    MutexedImage* prepareWindow(std::map<std::string,MutexedImage*>& images);
    MutexedImage* getImage(std::map<std::string,MutexedImage*>& images);

public:
    ImageWindow();
    ImageWindow(std::string imageName);
    ImageWindow(const ImageWindow&) = delete;
    ~ImageWindow(){
        _window.setActive(false);
    }

    std::string getImageName() const { return _imageName; }
    void setImageName(std::string imageName) {
        if(imageName != _imageName){
            _imageName = imageName;
            _mustPrepare = true;
        }
    }

    bool loop(std::map<std::string,MutexedImage*>& images);
};

#endif // IMAGEWINDOW_H
