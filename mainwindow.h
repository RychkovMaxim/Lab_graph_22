#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QSpinBox>
#include <vector>
#include <map>
#include <QTimer>
#include <queue>

const int INF = 1e9;

// Структура для хранения координат вершин при визуализации
struct NodePos {
    double x;
    double y;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onMatrixCellChanged(int row, int col);
    void addVertex();
    void removeVertex();
    void runBFS();
    void runDFS();
    void runDijkstra();
    void runFloyd();
    void processAnimationStep();

private:
    int V;
    std::vector<std::vector<int>> adjMatrix;
    std::vector<NodePos> nodePositions;

    QGraphicsScene *scene;
    QGraphicsView *view;
    QTableWidget *matrixTable;
    QTextEdit *logConsole;

    QSpinBox *startNodeSpin;
    QPushButton *btnAddVertex;
    QPushButton *btnRemoveVertex;
    QPushButton *btnBFS;
    QPushButton *btnDFS;
    QPushButton *btnDijkstra;
    QPushButton *btnFloyd;


    void initInitialGraph();
    void updateTableFromMatrix();
    void drawGraph();
    void drawArrow(QPointF start, QPointF end, QString label, QColor color);
    void log(QString message);

    QTimer *animationTimer;
    std::queue<int> animationQueue;
    int activeNode;

    std::vector<int> lastDijkstraDistances;
    bool isDijkstraRunning;

    bool isFloydRunning;
    std::vector<std::vector<std::vector<int>>> floydHistory;
};

#endif