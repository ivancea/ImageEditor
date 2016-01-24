#include <iostream>
#include <map>
#include <fstream>
#include <cmath>
#include <ctime>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include "Image.h"

using namespace std;

int main (int argc, char** argv) {
    srand(time(0));
    bool running = true;
    Image img;
    string fileName = "tigre.bmp";
    if(argc==2){
        fileName = argv[1];
    }else{
        string t;
        cout << "Insert fileName (default .bmp)(insert nothing for default \"" + fileName + "\"): ";
        getline(cin, t);
        if(t.size()>0){
            if(t.size()<4 || (t.substr(t.size()-4,4)!=".pbm" && t.substr(t.size()-4,4)!=".bmp"))
                t += ".bmp";
            fileName = t;
        }
        fileName = "images/" + fileName;
    }

    if((fileName.substr(fileName.size()-4,4)==".pbm" && !img.loadFromPBM(fileName))
    || (fileName.substr(fileName.size()-4,4)==".bmp" && !img.loadFromBMP(fileName))){
        cout << "Couldn't load file..." << endl;
        return 1;
    }

    map<sf::Keyboard::Key, void(*)(Image&)> keyBindings = {
        {sf::Keyboard::Q, [](Image& image){image.posterize(2);}},
        {sf::Keyboard::W, [](Image& image){image.blur(2);}},
        {sf::Keyboard::E, [](Image& image){image.bloom(5);}},
        {sf::Keyboard::R, [](Image& image){image.charcoal(20);}},
        {sf::Keyboard::T, [](Image& image){image.craze();}},
        {sf::Keyboard::A, [](Image& image){image.blackAndWhite();}},
        {sf::Keyboard::S, [](Image& image){image.grayscale();}},
        {sf::Keyboard::D, [](Image& image){image.invert();}},
        {sf::Keyboard::F, [](Image& image){image.modifyLight(2);}},
        {sf::Keyboard::G, [](Image& image){image.modifyLight(-2);}},
        {sf::Keyboard::Z, [](Image& image){image.cartoonize(2);}}
    };

    cout << "SPACE for save.\nENTER for reopen image.\nQ,W,E,R,T,A,S and D for apply effects." << endl;

    sf::Vector2i mouse;
    sf::RenderWindow window;
    window.create(sf::VideoMode(img.getX(), img.getY()), "ImagesEditor");
    window.setFramerateLimit(10);

    sf::Event ev;
    while(running){
        while(running && window.pollEvent(ev)){
            switch(ev.type){
            case sf::Event::MouseMoved:
                mouse.x = ev.mouseMove.x;
                mouse.y = ev.mouseMove.y;
                break;
            case sf::Event::MouseButtonPressed:
                SetFocus(window.getSystemHandle());
                break;
            case sf::Event::Closed:
                running = false;
                break;
            case sf::Event::KeyPressed:
                switch(ev.key.code){
                case sf::Keyboard::Space:
                    if(img.saveToPBM("resultado.pbm"))
                        cout << "Exported to \"resultado.pbm\"." << endl;
                    break;
                case sf::Keyboard::Return:
                    img.loadFromPBM(fileName);
                    break;
                case sf::Keyboard::Escape:
                    running = false;
                    break;
                default:
                    auto it = keyBindings.find(ev.key.code);
                    if(it!=keyBindings.end()){
                        it->second(img);
                    }
                    break;
                }
                break;
            default:
                break;
            }
        }
        window.clear();

        window.draw(img);

        window.display();
    }

    return 0;
}
