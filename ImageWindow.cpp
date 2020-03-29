#include "ImageWindow.hpp"

#include <windows.h>

std::map<sf::Keyboard::Key, void(*)(Image*&,void*)> keyBindings = {
    {sf::Keyboard::Q, [](Image*& image,void*){image->posterize(2);}},
    {sf::Keyboard::W, [](Image*& image,void*){image->blur(1);}},
    {sf::Keyboard::E, [](Image*& image,void*){image->bloom(5);}},
    {sf::Keyboard::R, [](Image*& image,void*){image->charcoal(20);}},
    {sf::Keyboard::T, [](Image*& image,void*){image->craze();}},
    {sf::Keyboard::A, [](Image*& image,void*){image->blackAndWhite();}},
    {sf::Keyboard::S, [](Image*& image,void*){image->grayscale();}},
    {sf::Keyboard::D, [](Image*& image,void*){image->invert();}},
    {sf::Keyboard::F, [](Image*& image,void*){image->modifyLight(2);}},
    {sf::Keyboard::G, [](Image*& image,void*){image->modifyLight(-2);}},
    {sf::Keyboard::Z, [](Image*& image,void*){image->cartoonize(2);}}
};

ImageWindow::ImageWindow()
:_mustPrepare(false){
}

ImageWindow::ImageWindow(std::string imageName)
:_imageName(imageName),_mustPrepare(false){
}

MutexedImage* ImageWindow::prepareWindow(std::map<std::string,MutexedImage*>& images){
    auto it = images.find(_imageName);
    MutexedImage* img = nullptr;
    if(it!=images.end()){
        img = it->second;
        sf::Vector2u size(img->getX(),img->getY());
        if(size.x<=0 || size.y<=0){
            if(_window.isOpen())
                _window.close();
            img = nullptr;
        }else if(!_window.isOpen()
        || _window.getSize().x != size.x
        || _window.getSize().y != size.y){
            sf::Vector2i position;
            if(_window.isOpen()){
                position = _window.getPosition();
                _window.close();
            }
            if(position.x+(int)size.x<=0){
                position.x = 0;
            }
            if(position.y<0)
                position.y = 0;
            _window.create(sf::VideoMode(size.x,size.y), ("Image: " + _imageName).c_str(), sf::Style::Close);
            _window.setFramerateLimit(10);
            _window.setPosition(position);
        }
    }else{
        _window.close();
        _imageName = "";
        _mouse.x = _mouse.y = 0;
    }
    _mustPrepare = false;
    return img;
}

MutexedImage* ImageWindow::getImage(std::map<std::string,MutexedImage*>& images){
    auto it = images.find(_imageName);
    MutexedImage* img = nullptr;
    if(it!=images.end()){
        img = it->second;
    }else{
        _window.close();
        _imageName = "";
        _mouse.x = _mouse.y = 0;
    }
    return img;
}

bool ImageWindow::loop(std::map<std::string,MutexedImage*>& images){
    bool running = true;
    sf::Event ev;
    MutexedImage* img = nullptr;
    if(_mustPrepare)
        img = prepareWindow(images);
    else
        img = getImage(images);
    if(img==nullptr)
        return false;
    while(running && _window.pollEvent(ev)){
        switch(ev.type){
        case sf::Event::MouseMoved:
            _mouse.x = ev.mouseMove.x;
            _mouse.y = ev.mouseMove.y;
            break;
        case sf::Event::MouseButtonPressed:
            SetFocus(_window.getSystemHandle());
            break;
        case sf::Event::Closed:
            running = false;
            break;
        case sf::Event::KeyPressed:
            switch(ev.key.code){
            case sf::Keyboard::Escape:
                running = false;
                break;
            default:
                auto it = keyBindings.find(ev.key.code);
                if(it!=keyBindings.end()){
                    img->call<void>(it->second, nullptr);
                }
                break;
            }
            break;
        default:
            break;
        }
    }

    if(running){
        _window.clear();
        _window.draw(*img);
        _window.display();
    }

    return !running;
}
