#include <iostream>
#include <map>
#include <vector>
#include <fstream>
#include <mutex>
#include <cmath>
#include <thread>
#include <chrono>

#include "Image.h"
#include "MutexedImage.h"
#include "ImageWindow.h"

using namespace std;

/// GLOBALS

map<string,MutexedImage*> images;
map<string,ImageWindow*> windows;

mutex globalMutex;

bool running = true;

string bindedImage,
       bindedWindow;

/// END GLOBALS

std::vector<std::string> split(std::string inicial, char busqueda = ' ', int maxc = -1){
    std::vector<std::string> temp;
    if(maxc==-1) maxc=inicial.size();
    if(inicial==""||maxc<2){
        temp.push_back(inicial);
        return temp;
    }
    for(int i=0; i<inicial.size()&&temp.size()<maxc-1;)
        if(inicial[i]==busqueda){
            if(i)
                temp.push_back(inicial.substr(0,i));
            inicial.erase(0, i+1);
            i=0;
        }else ++i;
    temp.push_back(inicial);
    for(int i=0; i<temp.size(); i++)
        if(temp[i]=="") temp.erase(temp.begin()+i);
    return temp;
}

void threadWindows(){
    while(running){
        globalMutex.lock();
        auto it = windows.begin();
        while(it!=windows.end()){
            if(it->second->loop(images)){
                delete it->second;
                it = windows.erase(it);
            }else it++;
        }
        globalMutex.unlock();
        this_thread::sleep_for(chrono::milliseconds(50));
    }
}

map<string,string> help = {
    {"help","Usage: help <command>"},
    {"exit","Usage: exit"},
    {"open","Usage: open <fileName>.(pbm|bmp)"},
    {"save","Usage: save <fileName>\n    *Saved as BMP*"},
    {"create","Usage: create (image|window) <varName>"},
    {"destroy","Usage: destroy (image|window) <varName>"},
    {"bind","Usage: bind (image|window) <varName>"},
    {"show","Usage: show (images|windows)"},
    {"join","Joins binded image and window"},
    {"unjoin","Unjoins binded window"}
};

bool interpret(string cmd, vector<string> args){
    if(cmd == "help"){
        if(args.size()==0){
            cout << "Current commands:" << endl;
            for(auto it:help)
                cout << " -" << it.first << endl;
        }else if(args.size()==1){
            auto it = help.find(args[0]);
            if(it!=help.end())
                cout << it->second << endl;
            else
                cout << "Unknown command" << endl;
        }else{
            cout << help[cmd] << endl;
        }
    }else if(cmd == "exit"){
        running = false;
    }else if(cmd == "show"){
        if(args.size()!=1){
            cout << help[cmd] << endl;
        }else{
            if(args[0]=="windows"){
                cout << "Windows(Image): " << windows.size() << endl;
                for(auto it:windows)
                    cout << " -" << it.first << "(" << it.second->getImageName() << ")" << endl;
            }else if(args[0]=="images"){
                cout << "Images: " << images.size() << endl;
                for(auto it:images)
                    cout << " -" << it.first << endl;
            }else{
                cout << help[cmd] << endl;
            }
        }
    }else if(cmd == "open"){
        if(args.size()!=1 || args[0].size()<4){
            cout << help[cmd] << endl;
        }else{
            if(bindedImage==""){
                cout << "Not image binded" << endl;
            }else if((args[0].substr(args[0].size()-4,4)==".pbm" && !images[bindedImage]->call<bool>([](Image*& image, void* data)->bool{return image->loadFromPBM(*(string*)data);}, &args[0]))
            || (args[0].substr(args[0].size()-4,4)==".bmp" && !images[bindedImage]->call<bool>([](Image*& image, void* data)->bool{return image->loadFromBMP(*(string*)data);}, &args[0]))){
                cout << "Couldn't open file..." << endl;
            }
        }
    }else if(cmd == "save"){
        if(args.size()!=1 || args[0].size()<4){
            cout << help[cmd] << endl;
        }else{
            if(bindedImage==""){
                cout << "Not image binded" << endl;
            }else if(!images[bindedImage]->call<bool>([](Image*& image, void* data)->bool{return image->saveToBMP(*(string*)data);}, &args[0])){
                cout << "Couldn't save file..." << endl;
            }
        }
    }else if(cmd == "bind"){
        if(args.size()!=2){
            cout << help[cmd] << endl;
        }else{
            if(args[0]=="window"){
                if(windows.find(args[1])!=windows.end())
                    bindedWindow = args[1];
                else
                    cout << "Inexistent window" << endl;
            }else if(args[0]=="image"){
                if(images.find(args[1])!=images.end())
                    bindedImage = args[1];
                else
                    cout << "Inexistent image" << endl;
            }else{
                cout << help[cmd] << endl;
            }
        }
    }else if(cmd == "create"){
        if(args.size()!=2){
            cout << help[cmd] << endl;
        }else{
            if(args[0]=="window"){
                if(windows.find(args[1])==windows.end()){
                    windows[args[1]] = new ImageWindow();
                }else
                    cout << "Existent window" << endl;
            }else if(args[0]=="image"){
                if(images.find(args[1])==images.end()){
                    images[args[1]] = new MutexedImage(new Image(), true);
                }else
                    cout << "Existent image" << endl;
            }else{
                cout << help[cmd] << endl;
            }
        }
    }else if(cmd == "destroy"){
        if(args.size()!=2){
            cout << help[cmd] << endl;
        }else{
            if(args[0]=="window"){
                auto it = windows.find(args[1]);
                if(it!=windows.end()){
                    lock_guard<mutex> _l(globalMutex);
                    delete it->second;
                    windows.erase(it);
                    bindedWindow = "";
                }else
                    cout << "Inexistent window" << endl;
            }else if(args[0]=="image"){
                auto it = images.find(args[1]);
                if(it!=images.end()){
                    lock_guard<mutex> _l(globalMutex);
                    it->second->setDeleteOnDestroy(true);
                    delete it->second;
                    images.erase(it);
                    cout << "Windows unjoined:";
                    for(auto p : windows){
                        if(p.second->getImageName()==args[1]){
                            cout << " " << p.first;
                            p.second->setImageName("");
                        }
                    }
                    cout << endl;
                    bindedImage = "";
                }else
                    cout << "Inexistent image" << endl;
            }else{
                cout << help[cmd] << endl;
            }
        }
    }else if(cmd == "join"){
        if(args.size()!=0){
            cout << help[cmd] << endl;
        }else{
            if(bindedImage==""){
                cout << "Not image binded" << endl;
            }else if(bindedWindow==""){
                cout << "Not window binded" << endl;
            }else{
                lock_guard<mutex> _l(globalMutex);
                windows[bindedWindow]->setImageName(bindedImage);
            }
        }
    }else if(cmd == "unjoin"){
        if(args.size()!=0){
            cout << help[cmd] << endl;
        }else{
            if(bindedWindow==""){
                cout << "Not window binded" << endl;
            }else{
                lock_guard<mutex> _l(globalMutex);
                ImageWindow* w = windows[bindedWindow];
                if(w->getImageName()=="")
                    cout << "Window not joined" << endl;
                else
                    w->setImageName("");
            }
        }
    }else{
        cout << "Unknown command. Type 'help' for see commands." << endl;
    }
    return false;
}

int main (int argc, char** argv) {
    srand(time(0));
    string fileName = "tigre.bmp";
    if(argc==2){
        fileName = argv[1];

        interpret("create",{"window","baseW"});
        interpret("create",{"image","baseI"});
        interpret("bind",{"window","baseW"});
        interpret("bind",{"image","baseI"});
        interpret("open",{fileName});
        interpret("join",{});
    }

    cout << "SPACE for save.\nENTER for reopen image.\nQ,W,E,R,T,A,S and D for apply effects." << endl;

    thread th(&threadWindows);

    while(running){
        string t;
        string joinedImage;
        auto it = windows.find(bindedWindow);
        if(it != windows.end())
            joinedImage = it->second->getImageName();
        cout << "[Img("<<bindedImage<<"),Wnd("<<bindedWindow<<")] >> ";
        getline(cin, t);
        vector<string> v = split(t);
        if(v.size()>0){
            string cmd = v[0];
            v.erase(v.begin());
            interpret(cmd, v);
        }
    }

    if(th.joinable())
        th.join();

    return 0;
}
