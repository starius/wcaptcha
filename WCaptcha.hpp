#ifndef WT_WCAPTCHA_H_
#define WT_WCAPTCHA_H_

#include <string>

#include <Wt/WCompositeWidget>
#include <Wt/WValidator>
#include <Wt/WImage>
#include <Wt/WFont>
#include <Wt/WLineEdit>
#include <Wt/WRasterImage>

namespace wcaptcha {
const char allowed[] = "23456789abcdeghknpqsuvxyz";
const int allowed_c = sizeof(allowed) - 1;
const int length = 5;
const int width = 140;
const int height = 60;
const int field = 10;
const int line_number = 3;
const float fluctuation_amplitude = 5;
const double transformation_delta = 0.2;
const bool show_credits = true;
const char* const credits = "server.com";
}

class WCaptcha : public Wt::WCompositeWidget {
public:
    WCaptcha(Wt::WContainerWidget* parent = 0);
    ~WCaptcha();
    Wt::WValidator::State validate();
    void update();

private:
    Wt::WContainerWidget* impl_;
    Wt::WImage* image_;
    Wt::WLineEdit* line_edit_;
    Wt::WRasterImage* raster_image_;

    std::string key_string_;

    static Wt::WFont random_font_();
    static Wt::WColor random_color_();
    void redraw_();
    void randomize_key_();
};

#endif // WT_WCAPTCHA_H_

