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

Point w_size(800, 800);

class AInteractor {
protected:
    AInteractor() {};
public:
    virtual ~AInteractor() = default;
    virtual void draw(QPainter&) = 0;
    virtual void move() = 0;
};

class AMovStrategy {
public:
    virtual ~AMovStrategy() = default;
    virtual Point calculateDelta(const Point& currentPosition) = 0;
};

class LinearMov : public AMovStrategy {
private:
    Point offset;
public:
    LinearMov(Point offset): offset(offset) {}
    Point calculateDelta(const Point& cur_po) override {
        return Point(cur_po.x+offset.x, cur_po.y+offset.y);
    }
};

class SinMovement : public AMovStrategy {
private:
    int counter;
    float phase;
    Point offset;
public:
    SinMovement(float phase, Point offset): offset(offset), phase(phase), counter(0) {}
    Point calculateDelta(const Point& cur_po) override {
        counter++;
        return Point(Point(cur_po.x+offset.x-3.*sin(float(counter)/phase), cur_po.y+offset.y));
    }
};

class DrawInteractor: public AInteractor {
public:
    Joint* model;
    QColor color;
    AMovStrategy* movementStrategy;
    DrawInteractor(QColor color, Point coords, AMovStrategy* strategy, std::stack<float> type = snow) {
        this->color = color;
        this->movementStrategy = strategy;
        model = new Joint(type, Point(coords.x, coords.y));
    }
    void draw(QPainter& paint) override {
        auto ptr = model;
        int c = 0;
        while (ptr!=nullptr) {
            paint.setPen(QColor(0, 0, 0, 0));
            paint.setBrush(color);
            Point first(ptr->getPoint().x-ptr->getR(), ptr->getPoint().y-ptr->getR());
            paint.drawEllipse(ptr->getPoint().x, ptr->getPoint().y, ptr->getR()*2., ptr->getR()*2.);
            if (ptr->next==nullptr) {break;}
            else ptr = ptr->next;
            c++;
        }
    }
    Point getPoint() {
        return model->getPoint();
    }
    void move() override {
        Point dir = movementStrategy->calculateDelta(model->getPoint());
        if (model->getPoint().y - model->getR() * 2 * model->size() > w_size.y) { dir = Point(model->getPoint().x, 0); }
        if (model->getPoint().x - model->getR() * 2 * model->size() >= w_size.x) { dir = Point(0, model->getPoint().y); }
        if (model->getPoint().x + model->getR() * 2 * model->size() <= 0) { dir = Point(w_size.x - 50 * (rand() % 12), 0); }
        if (model->getPoint().y + model->getR() * 2 * model->size() <= 0) { dir = Point(model->getPoint().x, w_size.y); }
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
    ~SceneManager() {
        for (auto i: Scenes) {
            delete i;
        }
    }
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
private:
    Scene* scene;
public:
    SceneConstructor() {
        scene = new Scene();
    }
    ~SceneConstructor() {
    }
    void createSinWorm(int count, float width, int len=1, QColor color = QColor(255,255,255)) {

        auto result = new Layer();
        std::stack<float> type;
        type.push(0.);
        for (int i=0;i<len;i++) {
            type.push(width);
        }
        for (int i=0;i<count;i++) {
            Point offset(-rand()%30/30., rand()%110/100.+1);
            result->addInteractor(new DrawInteractor(color, Point(rand()%int(w_size.x), rand()%int(w_size.y)), new SinMovement(rand()%30+15, offset), type));
        }
        scene->addLayer(result);
    }
    void createLinWorm(int count, float width, int len=1, QColor color = QColor(255,255,255)) {
        auto result = new Layer();
        std::stack<float> type;
        type.push(0.);
        for (int i=0;i<len;i++) {
            type.push(width);
        }
        for (int i=0;i<count;i++) {
            Point offset(-rand()%30/30., rand()%110/100.+1);
            result->addInteractor(new DrawInteractor(color, Point(rand()%int(w_size.x), rand()%int(w_size.y)), new LinearMov(offset), type));
        }
        scene->addLayer(result);
    }
    void createRoad() {
        auto result = new Layer();
        for (int i=0;i<2;i++) {
            result->addInteractor(new ImageInteractor(Point(i*w_size.x, w_size.y-200), Point(-4., 0.), dir+"road.png", Point(w_size.x, 200)));
        }
        scene->addLayer(result);
    }
    void createBiker() {
        auto result = new Layer();
        result->addInteractor(new ImageInteractor(Point(100, w_size.y-300), Point(0, 0.), dir+"biker.png", Point(200, 200)));
        scene->addLayer(result);
    }
    void createBuilds(Point size, float speed, int y=350) {
        auto result = new Layer();
        int count = w_size.x/size.x;
        for (int i=0;i<count;i++) {
            if (rand()%2==1) {
            result->addInteractor(new ImageInteractor(Point(i*(w_size.x/(count-1)), y), Point(speed, 0.), dir+"build"+QString().number(rand()%3+1)+".png", size));
            }
        }
        scene->addLayer(result);
    }
    void createBack() {
        auto result = new Layer();
        for (int i=0;i<2;i++) {
            result->addInteractor(new ImageInteractor(Point(i*w_size.x, 100), Point(-1., 0.), dir+"back.png", Point(w_size.x, 500)));
        }
        scene->addLayer(result);
    }
    void createBackMain(QString name = "backmain.png") {
        auto result = new Layer();
        result->addInteractor(new ImageInteractor(Point(0, 0), Point(0, 0.), dir+name, Point(w_size.x, w_size.y)));
        scene->addLayer(result);
    }
    void createBirds(int len, int width, int count, Point offset, int y_coord=0, Point b_offset = Point(5, 5)) {
        auto result = new Layer();
        std::stack<float> type;
        type.push(0.);
        for (int i=0;i<len;i++) {
            type.push(width);
        }
        for (int i=0;i<count;i++) {
            Point coord((i<count/2?b_offset.x*i:(count/2*b_offset.x)-b_offset.x*(i-count/2)), y_coord+i*b_offset.y);
            result->addInteractor(new DrawInteractor(QColor(100, 100, 100), coord, new LinearMov(offset), type));
        }
        scene->addLayer(result);
    }
    Scene* getScene() {return scene;}
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for(auto i: worm_form) worm.push(i);
    for(auto i: worm_form_2) snow.push(i);
}
MainWindow::~MainWindow()
{
    delete ui;
}

class Window {
private:
    SceneManager* SM;
    QLabel* label;
    QTimer* tmr;
public:
    void update() {
        QPixmap map(w_size.x, w_size.y);
        map.fill(QColor(30, 165, 255));
        QPainter paint(&map);
        SM->update(paint);
        label->setPixmap(map);
    }
    ~Window() {
        delete SM;
        delete label;
        delete tmr;
    }
    Window(int count) {
        QMainWindow* s = new QMainWindow();
        label = new QLabel(s);
        label->resize(w_size.x, w_size.y);
        s->resize(w_size.x, w_size.y);
        SM = new SceneManager();
        SceneConstructor* SC = new SceneConstructor();
        SC->createBackMain();
        SC->createBack();
        SC->createBuilds(Point(200, 200), -1., 300);
        SC->createBuilds(Point(100, 300), -2., 350);
        SC->createRoad();
        SC->createBiker();
        SM->addScene(SC->getScene(), 500);
        delete SC;
        SC = new SceneConstructor();
        SC->createBackMain("backm.png");
        SC->createSinWorm(15, 2, 2, QColor(255, 148, 26));
        SC->createBack();;
        SC->createBirds(2, 5, 10, Point(1, 0), 50, Point(13, 13));
        SC->createBuilds( Point(200, 200), -1., 300);
        SC->createBuilds( Point(100, 300), -2., 350);
        SC->createRoad();
        SC->createBiker();
        SC->createSinWorm(15, 4, 2, QColor(255, 148, 26));
        SM->addScene(SC->getScene(), 500);
        delete SC;
        SC = new SceneConstructor();
        SC->createBackMain("backm2.png");
        SC->createBack();
        SC->createLinWorm(50, 1);
        SC->createBuilds( Point(200, 200), -1., 300);
        SC->createLinWorm(50, 2);
        SC->createBuilds( Point(100, 300), -2., 350);
        SC->createRoad();
        SC->createBiker();
        SC->createLinWorm(25, 3);
        SM->addScene(SC->getScene(), 500);
        delete SC;
        SC = new SceneConstructor();
        SC->createBackMain("backm3.png");
        SC->createSinWorm(25, 2, 3, QColor(249, 130, 152));
        SC->createBack();
        SC->createBuilds( Point(200, 200), -1., 300);
        SC->createBuilds( Point(100, 300), -2., 350);
        SC->createRoad();
        SC->createBiker();
        SC->createSinWorm(10, 3, 3, QColor(249, 130, 152));
        SM->addScene(SC->getScene(), 500);
        delete SC;
        tmr = new QTimer(s);
        tmr->setInterval(count);
        QObject::connect(tmr, &QTimer::timeout, [this]() { this->update(); });
        s->show();
        tmr->start();
    }
};

void MainWindow::on_pushButton_clicked()
{
    Window* ws = new Window(ui->horizontalSlider->sliderPosition());
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->label->setText(QString().number(ui->horizontalSlider->sliderPosition())+" мс.");
}
