#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPolygonItem>
#include <QGraphicsTextItem>
#include <QLabel>
#include <QMessageBox>
#include <qmath.h>
#include <queue>
#include <stack>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    resize(1000, 700);
    setWindowTitle("Граф-Конструктор с алгоритмами");

    isDijkstraRunning = false;
    isFloydRunning = false;
    activeNode = -1;

    initInitialGraph();

    scene = new QGraphicsScene(this);
    view = new QGraphicsView(scene, this);
    view->setRenderHint(QPainter::Antialiasing);

    matrixTable = new QTableWidget(this);
    logConsole = new QTextEdit(this);
    logConsole->setReadOnly(true);
    logConsole->setPlaceholderText("Лог выполнения алгоритмов...");

    startNodeSpin = new QSpinBox(this);
    startNodeSpin->setPrefix("Старт: ");
    startNodeSpin->setRange(1, 6);

    btnAddVertex = new QPushButton("Добавить вершину", this);
    btnRemoveVertex = new QPushButton("Удалить вершину", this);
    btnBFS = new QPushButton("Обход в ширину (BFS)", this);
    btnDFS = new QPushButton("Обход в глубину (DFS)", this);
    btnDijkstra = new QPushButton("Дейкстра", this);
    btnFloyd = new QPushButton("Флойд-Уоршелл", this);

    QHBoxLayout *mainLayout = new QHBoxLayout();
    QVBoxLayout *leftLayout = new QVBoxLayout();
    QVBoxLayout *rightLayout = new QVBoxLayout();
    QHBoxLayout *btnLayout = new QHBoxLayout();

    btnLayout->addWidget(btnAddVertex);
    btnLayout->addWidget(btnRemoveVertex);
    btnLayout->addWidget(startNodeSpin);

    leftLayout->addLayout(btnLayout);
    leftLayout->addWidget(view, 3);
    leftLayout->addWidget(logConsole, 1);

    rightLayout->addWidget(new QLabel("Матрица смежности (INF = нет ребра):", this));
    rightLayout->addWidget(matrixTable);
    rightLayout->addWidget(btnBFS);
    rightLayout->addWidget(btnDFS);
    rightLayout->addWidget(btnDijkstra);
    rightLayout->addWidget(btnFloyd);

    mainLayout->addLayout(leftLayout, 2);
    mainLayout->addLayout(rightLayout, 1);

    QWidget *centralWidget = new QWidget(this);
    centralWidget->setLayout(mainLayout);
    setCentralWidget(centralWidget);

    connect(matrixTable, &QTableWidget::cellChanged, this, &MainWindow::onMatrixCellChanged);
    connect(btnAddVertex, &QPushButton::clicked, this, &MainWindow::addVertex);
    connect(btnRemoveVertex, &QPushButton::clicked, this, &MainWindow::removeVertex);
    connect(btnBFS, &QPushButton::clicked, this, &MainWindow::runBFS);
    connect(btnDFS, &QPushButton::clicked, this, &MainWindow::runDFS);
    connect(btnDijkstra, &QPushButton::clicked, this, &MainWindow::runDijkstra);
    connect(btnFloyd, &QPushButton::clicked, this, &MainWindow::runFloyd);

    updateTableFromMatrix();
    drawGraph();

    animationTimer = new QTimer(this);
    connect(animationTimer, &QTimer::timeout, this, &MainWindow::processAnimationStep);
}

MainWindow::~MainWindow() {}

void MainWindow::initInitialGraph() {
    V = 6;
    adjMatrix.assign(V, std::vector<int>(V, INF));
    for(int i=0; i<V; ++i) adjMatrix[i][i] = 0;

    adjMatrix[1][3] = 20;
    adjMatrix[4][1] = 21;
    adjMatrix[4][3] = 39;
    adjMatrix[1][0] = 28;
    adjMatrix[0][2] = 13;
    adjMatrix[2][4] = 30;

    adjMatrix[3][0] = 15;
    adjMatrix[3][5] = 31;
    adjMatrix[5][0] = 18;

    nodePositions = {
        {400, 350},
        {350, 100},
        {450, 250},
        {250, 220},
        {550, 150},
        {180, 400}
    };
}

void MainWindow::updateTableFromMatrix() {
    matrixTable->blockSignals(true);
    matrixTable->setRowCount(V);
    matrixTable->setColumnCount(V);

    QStringList headers;
    for(int i = 0; i < V; ++i) headers << QString::number(i + 1);
    matrixTable->setHorizontalHeaderLabels(headers);
    matrixTable->setVerticalHeaderLabels(headers);

    for(int i = 0; i < V; ++i) {
        for(int j = 0; j < V; ++j) {
            QTableWidgetItem *item = new QTableWidgetItem();
            if (adjMatrix[i][j] == INF) {
                item->setText("INF");
            } else {
                item->setText(QString::number(adjMatrix[i][j]));
            }
            matrixTable->setItem(i, j, item);
        }
    }
    matrixTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    matrixTable->blockSignals(false);
}

void MainWindow::onMatrixCellChanged(int row, int col) {
    QString val = matrixTable->item(row, col)->text().trimmed();
    if(val.toUpper() == "INF" || val.isEmpty()) {
        adjMatrix[row][col] = (row == col) ? 0 : INF;
    } else {
        bool ok;
        int num = val.toInt(&ok);
        if(ok) adjMatrix[row][col] = num;
    }
    drawGraph();
}

void MainWindow::addVertex() {
    V++;
    for(auto &row : adjMatrix) row.push_back(INF);
    adjMatrix.push_back(std::vector<int>(V, INF));
    adjMatrix[V-1][V-1] = 0;

    double centerX = 350.0;
    double centerY = 250.0;

    double phi = (V - 1) * 2.39996;

    double baseRadius = 140.0;
    double step = 15.0;
    double currentRadius = baseRadius + (V - 6) * step;

    if (V <= 6) {
        double angle = 2.0 * M_PI * (V - 1) / 6.0;
        nodePositions.push_back({centerX + baseRadius * cos(angle), centerY + baseRadius * sin(angle)});
    } else {
        nodePositions.push_back({centerX + currentRadius * cos(phi), centerY + currentRadius * sin(phi)});
    }

    updateTableFromMatrix();
    drawGraph();
    log(QString("Добавлена вершина %1").arg(V));
}

void MainWindow::removeVertex() {
    if(V <= 1) {
        QMessageBox::warning(this, "Ошибка", "Нельзя удалить последнюю вершину!");
        return;
    }
    V--;
    adjMatrix.erase(adjMatrix.begin() + V);
    for(auto &row : adjMatrix) row.pop_back();
    nodePositions.pop_back();

    updateTableFromMatrix();
    drawGraph();
    log(QString("Удалена последняя вершина %1").arg(V + 1));
}

void MainWindow::drawGraph() {
    scene->clear();
    int radius = 25;

    for(int i = 0; i < V; ++i) {
        for(int j = 0; j < V; ++j) {
            if(adjMatrix[i][j] != INF && i != j) {
                QPointF p1(nodePositions[i].x, nodePositions[i].y);
                QPointF p2(nodePositions[j].x, nodePositions[j].y);
                drawArrow(p1, p2, QString::number(adjMatrix[i][j]), Qt::darkCyan);
            }
        }
    }

    for(int i = 0; i < V; ++i) {
        double x = nodePositions[i].x;
        double y = nodePositions[i].y;

        QBrush brush = (i == activeNode) ? QBrush(Qt::yellow) : QBrush(Qt::white);
        QPen pen = (i == activeNode) ? QPen(Qt::red, 3) : QPen(Qt::black, 2);

        scene->addEllipse(x - radius, y - radius, radius * 2, radius * 2, pen, brush);

        QGraphicsTextItem *text = scene->addText(QString::number(i + 1));
        text->setFont(QFont("Arial", 12, QFont::Bold));
        text->setPos(x - text->boundingRect().width()/2, y - text->boundingRect().height()/2);
    }
}

void MainWindow::drawArrow(QPointF start, QPointF end, QString label, QColor color) {
    double radius = 25;
    double angle = qAtan2(end.y() - start.y(), end.x() - start.x());

    QPointF edgeStart = start + QPointF(radius * qCos(angle), radius * qSin(angle));
    QPointF edgeEnd = end - QPointF(radius * qCos(angle), radius * qSin(angle));

    scene->addLine(QLineF(edgeStart, edgeEnd), QPen(color, 2));

    double arrowSize = 12;
    QPointF arrowP1 = edgeEnd - QPointF(arrowSize * qCos(angle - M_PI / 6), arrowSize * qSin(angle - M_PI / 6));
    QPointF arrowP2 = edgeEnd - QPointF(arrowSize * qCos(angle + M_PI / 6), arrowSize * qSin(angle + M_PI / 6));

    QPolygonF arrowHead;
    arrowHead << edgeEnd << arrowP1 << arrowP2;
    scene->addPolygon(arrowHead, QPen(color), QBrush(color));

    QPointF midPoint = (edgeStart + edgeEnd) / 2.0;
    QGraphicsTextItem *text = scene->addText(label);
    text->setDefaultTextColor(Qt::red);
    text->setFont(QFont("Arial", 10, QFont::Bold));
    text->setPos(midPoint.x() - 10, midPoint.y() - 15);
}

void MainWindow::log(QString message) {
    logConsole->append(message);
}

void MainWindow::runBFS() {
    isDijkstraRunning = false;
    isFloydRunning = false;
    int start = startNodeSpin->value() - 1;

    animationTimer->stop();
    while(!animationQueue.empty()) animationQueue.pop();

    std::vector<bool> visited(V, false);
    std::queue<int> q;

    q.push(start);
    visited[start] = true;

    log("<b>Запуск BFS анимации...</b>");

    while(!q.empty()) {
        int curr = q.front();
        q.pop();

        animationQueue.push(curr);

        for(int i = 0; i < V; ++i) {
            if(adjMatrix[curr][i] != INF && curr != i && !visited[i]) {
                visited[i] = true;
                q.push(i);
            }
        }
    }

    animationTimer->start(800);
}

void MainWindow::runDFS() {
    isDijkstraRunning = false;
    isFloydRunning = false;
    int start = startNodeSpin->value() - 1;

    animationTimer->stop();
    while(!animationQueue.empty()) animationQueue.pop();

    std::vector<bool> visited(V, false);
    std::stack<int> s;

    s.push(start);

    log("<b>Запуск DFS анимации...</b>");

    while(!s.empty()) {
        int curr = s.top();
        s.pop();

        if(!visited[curr]) {
            visited[curr] = true;
            animationQueue.push(curr);
        }

        for(int i = V - 1; i >= 0; --i) {
            if(adjMatrix[curr][i] != INF && curr != i && !visited[i]) {
                s.push(i);
            }
        }
    }

    animationTimer->start(800);
}

void MainWindow::runDijkstra() {
    isFloydRunning = false;
    int start = startNodeSpin->value() - 1;

    animationTimer->stop();
    while(!animationQueue.empty()) animationQueue.pop();

    std::vector<int> dist(V, INF);
    std::vector<bool> visited(V, false);
    dist[start] = 0;

    log("<br><b> ЗАПУСК АЛГОРИТМА ДЕЙКСТРЫ </b>");
    log(QString("Ищем кратчайшие пути от вершины <b>%1</b>...").arg(start + 1));

    for (int i = 0; i < V; ++i) {
        int minDist = INF, u = -1;
        for (int j = 0; j < V; ++j) {
            if (!visited[j] && dist[j] <= minDist) {
                minDist = dist[j];
                u = j;
            }
        }
        if (u == -1) break;

        visited[u] = true;
        animationQueue.push(u);

        for (int v = 0; v < V; ++v) {
            if (!visited[v] && adjMatrix[u][v] != INF && u != v &&
                dist[u] + adjMatrix[u][v] < dist[v]) {
                dist[v] = dist[u] + adjMatrix[u][v];
            }
        }
    }

    lastDijkstraDistances = dist;
    isDijkstraRunning = true;

    animationTimer->start(1000);
}

void MainWindow::runFloyd() {
    animationTimer->stop();
    while(!animationQueue.empty()) animationQueue.pop();
    floydHistory.clear();

    isFloydRunning = true;
    isDijkstraRunning = false;

    auto currentDist = adjMatrix;

    log("<br><b>ЗАПУСК АЛГОРИТМА ФЛОЙДА-УОРШЕЛЛА</b>");
    log("Алгоритм находит кратчайшие пути между всеми парами вершин.");

    for (int k = 0; k < V; ++k) {
        for (int i = 0; i < V; ++i) {
            for (int j = 0; j < V; ++j) {
                if (currentDist[i][k] != INF && currentDist[k][j] != INF &&
                    currentDist[i][k] + currentDist[k][j] < currentDist[i][j]) {
                    currentDist[i][j] = currentDist[i][k] + currentDist[k][j];
                }
            }
        }

        floydHistory.push_back(currentDist);
        animationQueue.push(k);
    }

    animationTimer->start(1500);
}

void MainWindow::processAnimationStep() {
    if (animationQueue.empty()) {
        animationTimer->stop();
        activeNode = -1;
        drawGraph();

        if (isFloydRunning) {
            log("<br><b>Алгоритм Флойда завершен.</b>");
            isFloydRunning = false;
        }
        else if (isDijkstraRunning) {
            int start = startNodeSpin->value();
            QString resultText = QString("<br><b style='color:green;'>Итоговые кратчайшие расстояния от вершины %1:</b><br>").arg(start);

            for (int i = 0; i < V; ++i) {
                QString distStr;
                if (lastDijkstraDistances[i] == INF) {
                    distStr = "<span style='color:red;'>&infin; (нет пути)</span>";
                } else {
                    distStr = QString("<b style='color:darkgreen;'>%1</b>").arg(lastDijkstraDistances[i]);
                }
                resultText += QString("До вершины <b>%1</b> &rarr; Расстояние: %2<br>").arg(i + 1).arg(distStr);
            }
            log(resultText);
            isDijkstraRunning = false;
        }
        else {
            log("<span style='color:green;'><b>Обход завершен!</b></span>");
        }
        return;
    }

    activeNode = animationQueue.front();
    animationQueue.pop();

    if (isFloydRunning) {
        int stepK = activeNode;
        log(QString("<hr><b>Шаг %1: Используем вершину %2 как промежуточную</b>").arg(stepK + 1).arg(stepK + 1));

        auto matrix = floydHistory[stepK];
        QString htmlTable = "<table border='1' cellspacing='0' cellpadding='3' style='border-collapse: collapse; background-color: #f9f9f9;'>";

        htmlTable += "<tr style='background-color: #e0e0e0;'><td>&nbsp;</td>";
        for(int j=0; j<V; j++) htmlTable += QString("<td><b>%1</b></td>").arg(j+1);
        htmlTable += "</tr>";

        for (int i = 0; i < V; ++i) {
            htmlTable += "<tr>";
            htmlTable += QString("<td style='background-color: #e0e0e0;'><b>%1</b></td>").arg(i+1);
            for (int j = 0; j < V; ++j) {
                QString val = (matrix[i][j] == INF) ? "&infin;" : QString::number(matrix[i][j]);
                QString color = (i == stepK || j == stepK) ? "blue" : "black";
                htmlTable += QString("<td style='color: %1;'>%2</td>").arg(color, val);
            }
            htmlTable += "</tr>";
        }
        htmlTable += "</table>";
        log(htmlTable);
    }
    else if (isDijkstraRunning) {
        log(QString("Минимальное расстояние зафиксировано для вершины: <b style='color:darkblue;'>%1</b>").arg(activeNode + 1));
    }
    else {
        log(QString("Посещена вершина: <b style='color:blue;'>%1</b>").arg(activeNode + 1));
    }

    drawGraph();
}