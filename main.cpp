#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <vector>
#include <QImage>
#include <QDebug>
#include <exception>

QT_CHARTS_USE_NAMESPACE

std::vector<int> prepare_ideal_array(const std::vector<int>& array) {
    int min = 60;
    int max = 80;
    int ideal_value = 500;

    std::vector<int> ideal;
    ideal.reserve(array.size());

    for(int i = min; i < max; ++i) {
        ideal[i] = ideal_value;
    }

    return ideal;
}

double gsl_stats_correlation(const std::vector<int>& data)
{
    std::vector<int> ideal = prepare_ideal_array(data);
    const int stride1 = 1;
    const int stride2 = 1;

    double sum_xsq = 0.0;
    double sum_ysq = 0.0;
    double sum_cross = 0.0;

    double mean_x = data[0 * stride1];
    double mean_y = ideal[0 * stride2];

    for(int i = 1; i < data.size(); ++i) {
        double ratio = i / (i + 1.0);
        double delta_x = data[i * stride1] - mean_x;
        double delta_y = ideal[i * stride2] - mean_y;
        sum_xsq += delta_x * delta_x * ratio;
        sum_ysq += delta_y * delta_y * ratio;
        sum_cross += delta_x * delta_y * ratio;
        mean_x += delta_x / (i + 1.0);
        mean_y += delta_y / (i + 1.0);
    }

    double r = sum_cross / (sqrt(sum_xsq) * sqrt(sum_ysq));

    return r;
}

/*
std::vector<std::vector<int>> get_quad(const QPoint& point1, const QPoint& point2, const QImage& img) {
    std::vector<std::vector<int>> quad;
    for(int j = point1.x(); j < point2.x(); ++j) {
        std::vector<int> line;
        for(int i = point1.y(); i < point2.y(); ++i) {
            QRgb rgb = img.pixel(j, i);
            int average_y = (qRed(rgb) + qGreen(rgb) + qBlue(rgb)) / 3;
            line.append(average_y);
        }
        quad.append(line);
    }
    return quad;
}
*/
std::pair<std::vector<int>, std::vector<int>> append_vec(const QImage& img, const int q_num)
{
    int minX = 0;
    int maxX = 0;
    int minY = 0;
    int maxY = 0;

    switch (q_num) {
    case 1:
        minX = 0;
        maxX = img.width() / 2;
        minY = 0;
        maxY = img.height() / 2;
        break;
    case 2:
        minX = img.width() / 2;
        maxX = img.width();
        minY = 0;
        maxY = img.height() / 2;
        break;
    case 3:
        minX = 0;
        maxX = img.width() / 2;
        minY = img.height() / 2;
        maxY = img.height();
        break;
    case 4:
        minX = img.width() / 2;
        maxX = img.width();
        minY = img.height() / 2;
        maxY = img.height();
        break;
    default:
        break;
    }

    std::vector<int> result1;
    for(int x = minX; x < maxX; ++x) {
        int average = 0;
        for(int y = minY; y < maxY; ++y) {
            QRgb rgb = img.pixel(x, y);
            average += (qRed(rgb) + qGreen(rgb) + qBlue(rgb)) / 3;
        }
        result1.push_back(average / (maxY - minY));
    }


    std::vector<int> result2;
    for(int x = minY; x < maxY; ++x) {
        int average = 0;
        for(int y = minX; y < maxX; ++y) {
            QRgb rgb = img.pixel(y, x);
            average += (qRed(rgb) + qGreen(rgb) + qBlue(rgb)) / 3;
        }
        result2.push_back(average / (maxY - minY));
    }

    if(q_num == 1) {
        std::reverse(std::begin(result1), std::end(result1));
        std::reverse(std::begin(result2), std::end(result2));
    }

    if(q_num == 2) {
        std::reverse(std::begin(result2), std::end(result2));
    }

    if(q_num == 3) {
        std::reverse(std::begin(result1), std::end(result1));
    }

    return std::pair<std::vector<int>, std::vector<int>>(result1, result2);
}

typedef struct {
    std::pair<std::vector<int>, std::vector<int>> fist;
    std::pair<std::vector<int>, std::vector<int>> secn;
    std::pair<std::vector<int>, std::vector<int>> thir;
    std::pair<std::vector<int>, std::vector<int>> four;
} arrs_t;

arrs_t img2arr(const QImage& img) {
    int x0 = 0;
    int x1 = img.width() / 2;
    int x2 = img.width();
    int y0 = 0;
    int y1 = img.height() / 2;
    int y2 = img.height();

    arrs_t arrs;

    arrs.fist = append_vec(img, 1);
    arrs.secn = append_vec(img, 2);
    arrs.thir = append_vec(img, 3);
    arrs.four = append_vec(img, 4);

    return arrs;
}

QPair<QLineSeries*, QLineSeries*> make_series(const std::vector<std::vector<int>>& quad)
{
    QLineSeries *line_horizontal = new QLineSeries();
    QLineSeries *line_vertical = new QLineSeries();

    line_horizontal->setName("horizontal");
    line_vertical->setName("vertical");

    for(int i = 0; i < quad.size(); ++i) {
        int average = 0;
        for(int j = 0; j < quad[i].size(); ++j) {
            average += quad[i][j];
        }
        line_horizontal->append(i, average / quad[i].size());
    }

    for(int i = 0; i < quad[i].size(); ++i) {
        int average = 0;

        for(int j = 0; j < quad.size(); ++j) {
            average += quad[j][i];
        }
        line_vertical->append(i, average / quad.size());
    }

    return QPair<QLineSeries*, QLineSeries*>(line_horizontal, line_vertical);
}

QLineSeries* make_ser(const std::vector<int>& arr) {
    auto s = new QLineSeries();
    for(size_t i = 0; i < arr.size(); ++i) {
        s->append(i, arr.at(i));
    }
    return s;
}


int main(int argc, char *argv[])
{
    QImage img;

    qDebug() << "image is loaded:" << img.load(":/imgs/good_0.png");
    qDebug() << "height:" << img.height();
    qDebug() << "width:" << img.width();

    arrs_t arrs = img2arr(img);

    QApplication a(argc, argv);

    QChart *chart = new QChart();
    chart->legend()->hide();

    auto s1 = make_ser(arrs.fist.first);
    chart->addSeries(s1);

    auto s2 = make_ser(arrs.fist.second);
    chart->addSeries(s2);

    auto s3 = make_ser(arrs.secn.first);
    chart->addSeries(s3);

    auto s4 = make_ser(arrs.secn.second);
    chart->addSeries(s4);

    auto s5 = make_ser(arrs.thir.first);
    chart->addSeries(s5);

    auto s6 = make_ser(arrs.thir.second);
    chart->addSeries(s6);

    auto s7 = make_ser(arrs.four.first);
    chart->addSeries(s7);

    auto s8 = make_ser(arrs.four.second);
    chart->addSeries(s8);


    chart->createDefaultAxes();

    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    QMainWindow window;
    window.setCentralWidget(chartView);
    window.resize(800, 500);
    window.show();

    return a.exec();

}
