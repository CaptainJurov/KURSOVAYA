#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "joint.h"
#include <vector>
#include <cmath>
#include <stack>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QImage>
QString dir = "../../";
std::vector<float> worm_form = {2., 4., 5., 6., 7., 8., 9., 10., 11., 13., 15., 17., 20., 21., 23., 25., 25, 25, 25, 25, 25, 25, 25., 24., 20.};
std::stack<float> worm;
std::vector<float> worm_form_2 = {0., 5.};
std::stack<float> snow;
int T = 0;
Point w_size(800, 800);



class AInteractor {
protected:
    AInteractor() {};
public:
    virtual ~AInteractor() = default;
    virtual void draw(QPainter&) = 0;
    virtual void move() = 0;
};

class DrawInteractor: public AInteractor {
public:
    Point offset;
    Joint* model;
    QColor color;
    float phase;
    int counter = 0;
    DrawInteractor(QColor color, Point offset, Point coords, float phase = 15., std::stack<float> type = snow): phase(phase), offset(offset) {
        this->color = color;
        model = new Joint(type, Point(coords.x, coords.y));
    }
    void draw(QPainter& paint) override {
        auto ptr = model;
        int c = 0;
        while (ptr!=nullptr) {
            paint.setPen(QColor(0, 0, 0, 0));
            paint.setBrush(color);
            Point first(ptr->getPoint().x-ptr->getR(), ptr->getPoint().y-ptr->getR());
            paint.drawEllipse(ptr->getPoint().x, ptr->getPoint().y, ptr->getR()*2, ptr->getR()*2);
            if (ptr->next==nullptr) {break;}
            else ptr = ptr->next;
            c++;
        }
    }
    Point getPoint() {
        return model->getPoint();
    }
    void move() override {
        counter++;
        Point dir(Point(model->getPoint().x+offset.x-3*sin(counter/phase), model->getPoint().y+offset.y));
        if (model->getPoint().y>w_size.y) {dir = Point(model->getPoint().x, 0);}
        else if (model->getPoint().x+model->getR()*2>=w_size.x) {dir = Point(model->getPoint().x+50*(rand()%12), model->getPoint().y);}
        else if (model->getPoint().x+model->getR()*2<=0) {dir = Point(w_size.x-50*(rand()%12), 0);}
        else if (model->getPoint().y+model->getR()<=0) {dir = Point(model->getPoint().x, w_size.y);}
        model->move(dir);
    }
};

class ImageInteractor: public AInteractor {
private:
    Point offset;
    Point coords;
    QString file;
    Point size;
    QImage Image;
public:
    ImageInteractor(Point coords,Point offset, QString name, Point size): size(size), coords(coords), offset(offset), file(name) {
        Image = QImage(name);
        Image = Image.scaled(size.x, size.y);
    }
    void draw(QPainter& painter) override {
        painter.drawImage(coords.x, coords.y, Image);
    }
    void move() override {
        coords = Point(coords.x+offset.x, coords.y);

        if (coords.x>w_size.x+size.x) {coords = Point(10, coords.y);}
        if (coords.x+size.x<=0) {coords = Point(w_size.x, coords.y);}
        if (coords.y>w_size.y+size.y) {coords= Point(coords.x, 100);}
        if (coords.y+size.y<=0) {coords= Point(coords.x, w_size.y);}
    }
};

struct Layer {
public:
    std::vector<AInteractor*> interactors;
    Layer() {}
    Layer(std::vector<AInteractor*> inter) {interactors=inter;}
    void update(QPainter& paint) {
        for (auto interactor : interactors) {
            interactor->move();
            interactor->draw(paint);
        }
    }
    void addInteractor(AInteractor* inter) {
        interactors.push_back(inter);
    }
};

class Scene {
public:
    std::vector<Layer*> layers;
    Scene() {}
    void update(QPainter& paint) {
        for (auto layer : layers) {
            layer->update(paint);
        }
    }
    void addLayer(Layer* layer) {layers.push_back(layer);}
};

class SceneManager {
public:
    std::vector<Scene*> Scenes;
    std::vector<int> sceneDurations;
    int activeSceneIndex;
    int currentTick;

    SceneManager() : activeSceneIndex(0), currentTick(0) {}

    void addScene(Scene* scene, int duration) {
        Scenes.push_back(scene);
        sceneDurations.push_back(duration);
    }

    void setActiveScene(int index) {
        if (index >= 0 && index < Scenes.size()) {
            activeSceneIndex = index;
            currentTick = 0;
        }
    }

    void update(QPainter& paint) {
        if (activeSceneIndex >= 0 && activeSceneIndex < Scenes.size()) {
            Scenes[activeSceneIndex]->update(paint);
            currentTick++;
            if (currentTick >= sceneDurations[activeSceneIndex]) {
                activeSceneIndex = (activeSceneIndex + 1) % Scenes.size();
                currentTick = 0;
            }
        }
    }
};

class SceneConstructor {
public:
    SceneConstructor() {}
    static Layer* createWorm(int count, float width, int len=1, QColor color = QColor(255, 255,255)) {
        auto result = new Layer();
        std::stack<float> type;
        type.push(0.);
        for (int i=0;i<len;i++) {
            type.push(width);
        }
        for (int i=0;i<count;i++) {
            result->addInteractor(new DrawInteractor(color, Point(-rand()%30/30., rand()%110/100.+1), Point(rand()%int(w_size.x), rand()%int(w_size.y)), rand()%30+15, type));
        }
        return result;
    }
    static Layer* createRoad() {
        auto result = new Layer();
        for (int i=0;i<2;i++) {
            result->addInteractor(new ImageInteractor(Point(i*w_size.x, w_size.y-200), Point(-4., 0.), dir+"road.png", Point(w_size.x, 200)));
        }
        return result;
    }
    static Layer* createBiker() {
        auto result = new Layer();
        result->addInteractor(new ImageInteractor(Point(100, w_size.y-300), Point(0, 0.), dir+"biker.png", Point(200, 200)));
        return result;
    }
    static Layer* createBuilds(Point size, float speed, int y=350) {
        auto result = new Layer();
        int count = w_size.x/size.x;
        for (int i=0;i<count;i++) {
            if (rand()%2==1) {
            result->addInteractor(new ImageInteractor(Point(i*(w_size.x/(count-1)), y), Point(speed, 0.), dir+"build"+QString().number(rand()%3+1)+".png", size));
            }
        }
        return result;
    }
    static Layer* createBack() {
        auto result = new Layer();
        for (int i=0;i<2;i++) {
            result->addInteractor(new ImageInteractor(Point(i*w_size.x, 100), Point(-1., 0.), dir+"back.png", Point(w_size.x, 500)));
        }
        return result;
    }
    static Layer* createBackMain(QString name = "backmain.png") {
        auto result = new Layer();
        result->addInteractor(new ImageInteractor(Point(0, 0), Point(0, 0.), dir+name, Point(w_size.x, w_size.y)));
        return result;
    }
    static Layer* createBirds() {
        auto result = new Layer();
        result->addInteractor(new ImageInteractor(Point(0, 0), Point(0, 0.), dir+"backmain.png", Point(w_size.x, w_size.y)));
        return result;
    }
};

SceneManager* SM;
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    tmr = new QTimer();
    tmr->setInterval(15);
    connect(tmr, SIGNAL(timeout()), this, SLOT(updateTime()));
    tmr->start();
    for(auto i: worm_form) worm.push(i);
    for(auto i: worm_form_2) snow.push(i);

    SM = new SceneManager();
    auto scene3 = new Scene();
    scene3->addLayer(SceneConstructor::createBackMain());
    scene3->addLayer(SceneConstructor::createBack());;
    scene3->addLayer(SceneConstructor::createBuilds( Point(200, 200), -1., 300));
    scene3->addLayer(SceneConstructor::createBuilds( Point(100, 300), -2., 350));
    scene3->addLayer(SceneConstructor::createRoad());
    scene3->addLayer(SceneConstructor::createBiker());
    SM->addScene(scene3, 100);
    auto scene = new Scene();
    auto scene2 = new Scene();
    scene2->addLayer(SceneConstructor::createBackMain("backm.png"));
    scene2->addLayer(SceneConstructor::createWorm(15, 2, 2, QColor(255, 148, 26)));
    scene2->addLayer(SceneConstructor::createBack());;
    scene2->addLayer(SceneConstructor::createBuilds( Point(200, 200), -1., 300));
    scene2->addLayer(SceneConstructor::createBuilds( Point(100, 300), -2., 350));
    scene2->addLayer(SceneConstructor::createRoad());
    scene2->addLayer(SceneConstructor::createWorm(15, 4, 2, QColor(255, 148, 26)));
    scene2->addLayer(SceneConstructor::createBiker());
    SM->addScene(scene2, 100);
    scene->addLayer(SceneConstructor::createBackMain("backm2.png"));
    scene->addLayer(SceneConstructor::createBack());
    scene->addLayer(SceneConstructor::createWorm(50, 1));
    scene->addLayer(SceneConstructor::createBuilds( Point(200, 200), -1., 300));
    scene->addLayer(SceneConstructor::createWorm(50, 2));
    scene->addLayer(SceneConstructor::createBuilds( Point(100, 300), -2., 350));
    scene->addLayer(SceneConstructor::createRoad());
    scene->addLayer(SceneConstructor::createWorm(25, 3));
    scene->addLayer(SceneConstructor::createBiker());
    SM->addScene(scene, 100);
    auto scene4 = new Scene();
    scene4->addLayer(SceneConstructor::createBackMain("backm3.png"));
    scene4->addLayer(SceneConstructor::createWorm(25, 2, 3, QColor(249, 130, 152)));
    scene4->addLayer(SceneConstructor::createBack());;
    scene4->addLayer(SceneConstructor::createBuilds( Point(200, 200), -1., 300));
    scene4->addLayer(SceneConstructor::createBuilds( Point(100, 300), -2., 350));
    scene4->addLayer(SceneConstructor::createRoad());
    scene4->addLayer(SceneConstructor::createWorm(10, 3, 3, QColor(249, 130, 152)));
    scene4->addLayer(SceneConstructor::createBiker());
    SM->addScene(scene4, 100);
    // auto scene5 = new Scene();
    // scene5->addLayer(SceneConstructor::createBackMain());
    // scene5->addLayer(SceneConstructor::createWorm(5, 5, 20, QColor(58, 255, 74)));
    // scene5->addLayer(SceneConstructor::createBack());;
    // scene5->addLayer(SceneConstructor::createBuilds( Point(200, 200), -1., 300));
    // scene5->addLayer(SceneConstructor::createBuilds( Point(100, 300), -2., 350));
    // scene5->addLayer(SceneConstructor::createRoad());
    // scene5->addLayer(SceneConstructor::createWorm(5, 10, 20, QColor(58, 255, 74)));
    // SM->addScene(scene5, 1000);


}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateTime()
{
    T++;
    QPixmap map(w_size.x, w_size.y);
    map.fill(QColor(30, 165, 255));
    //auto Image = QImage("/home/frogget/Изображения/ptica.jpg");
    //auto Image = QImage(dir+"build.png");
    //Image = Image.scaled(w_size.x, w_size.y);
    QPainter paint(&map);
    //paint.drawImage(0,0,Image);
    SM->update(paint);
    ui->label->setText(QString().number(SM->activeSceneIndex));
    ui->label_2->setPixmap(map);
}

