
#include <iostream>
#include <algorithm>

#include <boost/regex.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <Wt/WPainter>
#include <Wt/WPen>
#include <Wt/WPushButton>
#include <Wt/WColor>
#include <Wt/WContainerWidget>
#include <Wt/WPainterPath>
#include <Wt/WPoint>
#include <Wt/WTransform>

#include "WCaptcha.hpp"

boost::regex bad("cp|cb|ck|c6|c9|rn|rm|mm|co|do|cl|db|qp|qb|dp|ww");

class WCaptchaRandom : public boost::mt19937 {
public:
    WCaptchaRandom() : boost::mt19937() {
        long s = boost::posix_time::microsec_clock::universal_time()
                 .time_of_day().total_microseconds();
        seed(s);
    }

    int rr(int start, int stop, int step = 1) {
        return start + (((*this)()) % ((stop - start) / step)) * step;
    }

    double drr(double start, double stop) {
        double stop_start = stop - start;
        double max_min = max() - min();
        return start + double((*this)()) / max_min * stop_start;
    }
} r_;

WCaptcha::WCaptcha(Wt::WContainerWidget* parent) :
    Wt::WCompositeWidget(parent), key_string_(wcaptcha::length, ' ') {
    setImplementation(impl_ = new Wt::WContainerWidget());
    // raster_image_->setPixel(10, 10, Wt::black);
    image_ = new Wt::WImage(impl_);
    line_edit_ = new Wt::WLineEdit(impl_);
    line_edit_->setTextSize(wcaptcha::length);
    Wt::WPushButton* update_b = new Wt::WPushButton(tr("wcaptcha.refresh"), impl_);
    update_b->clicked().connect(this, &WCaptcha::update);
    update();
    // Wt::WRasterImage* r = new Wt::WRasterImage(100, 100);
    // Wt::WImage* test_image = new Wt::WImage(r, "", this);
    // Wt::WPainter painter(r);
    // paiter.
}

Wt::WFont WCaptcha::random_font_() {
    using Wt::WFont;
    WFont font;
    int n;
    WFont::Style const styles[] =
    {WFont::NormalStyle, WFont::Italic, WFont::Oblique};
    const int styles_c = sizeof(styles) / sizeof(WFont::Style);
    n = r_() % styles_c;
    font.setStyle(styles[n]);
    WFont::Variant const variants[] =
    {WFont::NormalVariant, WFont::SmallCaps};
    const int variants_c = sizeof(variants) / sizeof(WFont::Variant);
    n = r_() % variants_c;
    font.setVariant(variants[n]);
    WFont::GenericFamily const families[] =
    { WFont::SansSerif, WFont::Cursive };
    const int families_c = sizeof(families) / sizeof(WFont::GenericFamily);
    n = r_() % families_c;
    font.setFamily(families[n]);
    double s = r_.drr(45, 50);
    font.setSize(WFont::FixedSize, s);
    n = r_.rr(100, 900);
    font.setWeight(WFont::Value, n);
    return font;
}

void WCaptcha::randomize_key_() {
    while (true) {
        for (int i = 0; i < wcaptcha::length; i++) {
            int n = r_() % wcaptcha::allowed_c;
            std::cout << wcaptcha::allowed_c << std::endl;
            key_string_[i] = wcaptcha::allowed[n];
        }
        if (!boost::regex_search(key_string_, bad)) {
            break;
        }
    }
}

Wt::WColor WCaptcha::random_color_() {
    int sum = 255;
    int rgb[3];
    for (int i = 0; i < 3; i++) {
        rgb[i] = r_.rr(0, sum);
        sum -= rgb[i];
    }
    return Wt::WColor(rgb[0], rgb[1], rgb[2]);
}

void WCaptcha::redraw_() {
    raster_image_ = new Wt::WRasterImage("png",
                                         wcaptcha::width, wcaptcha::height);//m leaks?
    image_->setResource(raster_image_);
    Wt::WPainter painter(raster_image_);
    //painter.rotate(30);
    // painter.setWorldTransform(Wt::WTransform(1,0.5,0.5,1,0,0), true);
    //painter.translate(0, -wcaptcha::height);
    Wt::WColor color = random_color_();
    painter.setPen(Wt::WPen(color));
    // painter.setBrush(Wt::WBrush(color));
    Wt::WFont font = random_font_();
    painter.setFont(font);
    double x = wcaptcha::field;
    double x_step = (wcaptcha::width - wcaptcha::field) /
                    (wcaptcha::length + 1);
    //x_step += r_.drr(-x_step/10, x_step/10);
    double font_height = font.fixedSize().value();
    double y_max = wcaptcha::height - font_height;
    for (int i = 0; i < wcaptcha::length; i++) {
        std::cout << "test" << std::endl;
        int y = r_.drr(0, y_max);
        //int y = 5;
        std::string s(1, key_string_[i]);
        std::cout << s << std::endl;
        //double x_offset = x + x_step/2;
        //double y_offset = y + font_height/2;
        painter.resetTransform();
        painter.setWorldTransform(Wt::WTransform(1, 0, 0, 1, x, y));
        //painter.rotate(r_.drr(-10, 10));
        const double d = wcaptcha::transformation_delta;
        double m11 = r_.drr(1 - d, 1 + d);
        double m12 = r_.drr(-d, d);
        double m21 = r_.drr(-d, d);
        double m22 = r_.drr(1 - d, 1 + d);
        std::cout << m11 << " " << m12 << " " << m21 << " " << m22 << std::endl;
        painter.setWorldTransform(Wt::WTransform(m11, m12, m21, m22, 0, 0), true);
        //painter.setWorldTransform(Wt::WTransform(0,1,-1,0,0,0), true);
        // painter.setWorldTransform(Wt::WTransform(1,0,0,1,x_offset,y_offset), true);
        painter.drawText(0, 0, wcaptcha::width, wcaptcha::height,
                         Wt::AlignTop | Wt::AlignLeft, s);
        x += x_step; // + r_.drr(-x_step/10, x_step/10);
    }
    painter.setWorldTransform(Wt::WTransform());
    Wt::WColor line_color;
    for (int i = 0; i < wcaptcha::line_number; i++) {
        for (int c = 0; c < 1; c++) {
            // first: lines with same color, second: white lines
            line_color = c ? Wt::white : color;
            Wt::WPen pen(line_color);
            // pen.setWidth(font.weightValue() / 200.0);
            pen.setWidth(2.5);
            painter.setPen(pen);
            using Wt::WPoint;
            int w = wcaptcha::width / 4;
            int h = wcaptcha::height;
            double from_x = r_.drr(0, w);
            double from_y = r_.drr(0, h);
            double c1_x = r_.drr(w, 2 * w);
            double c1_y = r_.drr(0, h);
            double c2_x = r_.drr(2 * w, 3 * w);
            double c2_y = r_.drr(0, h);
            double ep_x = r_.drr(3 * w, 4 * w);
            double ep_y = r_.drr(0, h);
            // if (c == 0 && r_.rr(0,2) == 0)
            // {
            //     std::swap(from_x, ep_x);
            // }
            // if (c == 0 && r_.rr(0,2) == 0)
            // {
            //     std::swap(from_x, c1_x);
            // }
            Wt::WPainterPath pp;
            pp.moveTo(from_x, from_y);
            pp.cubicTo(c1_x, c1_y, c2_x, c2_y, ep_x, ep_y);
            // pp.quadTo(c1_x, c1_y, ep_x, ep_y);
            // pp.lineTo(ep_x, ep_y);
            painter.drawPath(pp);
        }
    }
    // painter.end();
    // Wt::WPainter painter1(raster_image_);
    // painter1.rotate(180.0);
    // painter1.end();
}

void WCaptcha::update() {
    randomize_key_();
    redraw_();
    line_edit_->setText("");
}

Wt::WValidator::State WCaptcha::validate() {
    if (line_edit_->text() == key_string_) {
        return  Wt::WValidator::Valid;
        // clear();
    } else if (line_edit_->text().empty()) {
        std::cout << 12345 << std::endl;
        return  Wt::WValidator::InvalidEmpty;
    } else {
        std::cout << 666 << line_edit_->text() << 666 << std::endl;
        update();
        return  Wt::WValidator::Invalid;
    }
}

// class WCaptcha : public Wt::WCompositeWidget
// {
// public:
//     WCaptcha(Wt::WContainerWidget *parent = 0);
//     Wt::WValidator::State validate();
//     void update();
// private:
//     Wt::ContainerWidget* impl_;
//     Wt::WImage* image_;
//     Wt::WLineEdit* line_edit_;
// };
WCaptcha::~WCaptcha() {
    //this->Wt::WCompositeWidget::~WCompositeWidget();
    delete raster_image_;
    //delete impl_;
}

