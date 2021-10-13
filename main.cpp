#include <gtkmm.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;
using namespace Gtk;

Glib::RefPtr<Gdk::Pixbuf> original_image;
Glib::RefPtr<Gdk::Pixbuf> image;

int allocated_width, allocated_height;
int scaled_width, scaled_height;
double scaling_factor = 1;
double x_center = 0, y_center = 0;
bool empty = true;

void set_dimensions(int &scaled_width , int &scaled_height , int available_width , int available_height , int original_width , int original_height){ 
    scaled_width = allocated_width;
    scaled_height = (scaled_width * original_height) / original_width;
    if (scaled_height > allocated_height)
    {
        scaled_height = allocated_height;
        scaled_width = (scaled_height * original_width) / original_height;
    }

    scaled_width *= scaling_factor;
    scaled_height *= scaling_factor;
}

std::string get_arguments(int argc , char* argv[]){
    std::string text = "";
    if(argc < 2){
        return "";
    }
    for(int i=1 ; i<argc ; i++){
        text.append(argv[i]);
    }
    return text;
}

int main(int argc , char* argv[]){
    fs::path file = get_arguments(argc , argv);
    if(argc < 2){
        std::cout << "usage:\n view [path]" << std::endl;
        return 1;
    }
    if(!fs::exists(file)){
        std::cout << file << " Not Found" << std::endl;
        return 1;
    }

    auto app = Application::create();
    auto screen = Gdk::Screen::get_default();

    Window window;  
    window.set_size_request(-1 , 300);
    window.set_default_size(600 , 400);
    window.set_icon_name("image-viewer");

    HeaderBar titlebar;
    window.set_titlebar(titlebar);
    titlebar.set_title(file.filename().string());
    titlebar.set_show_close_button(true);

    Button menu_button , zoom_button , edit_button;
    menu_button.set_image_from_icon_name("application-menu");
    zoom_button.set_image_from_icon_name("zoom");
    edit_button.set_image_from_icon_name("document-edit");

    titlebar.pack_start(menu_button);
    titlebar.pack_start(zoom_button);
    titlebar.pack_end(edit_button);

    ModelButton details_item , about_item;
    details_item.set_label("Details");
    about_item.set_label("About");

    VBox menu_box;
    menu_box.set_border_width(5);
    menu_box.pack_start(details_item , PACK_EXPAND_WIDGET);
    menu_box.pack_start(about_item , PACK_EXPAND_WIDGET);

    Popover popover;
    popover.set_relative_to(menu_button);
    popover.add(menu_box);
    popover.show_all_children();

    menu_button.signal_clicked().connect([&](){
        popover.popup();
    });

    Scale zoom_scale;
    zoom_scale.set_range(1 , 2.5);
    zoom_scale.set_size_request(100);
    zoom_scale.property_margin() = 5;

    Popover scroll_popover;
    scroll_popover.add(zoom_scale);
    scroll_popover.set_relative_to(zoom_button);
    scroll_popover.show_all_children();

    zoom_button.signal_clicked().connect([&](){
        scroll_popover.popup();
    });

    ScrolledWindow scrolled_window;
    window.add(scrolled_window);

    //----------------The Image---------------------------------------
    auto original_pixbuf = Gdk::Pixbuf::create_from_file(file);
    auto scaled_pixbuf = original_pixbuf;
    Image image(scaled_pixbuf);
    VBox box;
    box.pack_start(image , PACK_EXPAND_PADDING);
    scrolled_window.add(box);
    //allocated_width = window.get_allocation().get_width();
    //allocated_height = window.get_allocation().get_height();
    //std::cout << allocated_width << " " <<  allocated_height << std::endl;
    //scaled_pixbuf = original_pixbuf->scale_simple(allocated_width , allocated_height , Gdk::InterpType::INTERP_BILINEAR);
    //image.set(scaled_pixbuf);
    window.signal_size_allocate().connect([&](Allocation& a){
        allocated_width = box.get_allocation().get_width();
        allocated_height = box.get_allocation().get_height();
        set_dimensions(scaled_width , scaled_height , allocated_width , allocated_height , original_pixbuf->get_width() , original_pixbuf->get_height());
        //std::cout << allocated_width << " " <<  allocated_height << std::endl;
        scaled_pixbuf = original_pixbuf->scale_simple(scaled_width , scaled_height , Gdk::InterpType::INTERP_BILINEAR);
        image.set(scaled_pixbuf);
        scrolled_window.queue_compute_expand();
        scrolled_window.queue_resize();
    });

    zoom_scale.signal_change_value().connect([&](ScrollType type , double value){
        scaling_factor = zoom_scale.get_value();
        image.queue_draw();
        scrolled_window.queue_resize();
        scrolled_window.queue_compute_expand();
        return true;
    });
    //window.signal_size_allocate().connect([&](Gtk::Allocation&){
        //allocated_width = scrolled_window.get_allocation().get_width();
        //allocated_height = scrolled_window.get_allocation().get_height();
        //set_dimensions();
        //scaled_pixbuf = original_pixbuf->scale_simple(scaled_width , scaled_height , Gdk::InterpType::INTERP_BILINEAR); 
        //image.set(scaled_pixbuf);
    //});
    //scrolled_window.signal_draw().connect([&](const Cairo::RefPtr<Cairo::Context>){

    //});
    //----------------------------------------------------------------

    //----------------Details Window-----------------------------------
    Window details_window;
    details_window.set_resizable(false);

    HeaderBar details_window_title_bar;
    details_window.set_titlebar(details_window_title_bar);
    details_window_title_bar.set_title("Details");
    details_window_title_bar.set_show_close_button(true);

    VBox details_box;
    details_box.set_border_width(10);
    Label lname("Name: "+file.filename().string()) 
        , lpath("Path: "+file.string())
        , ldimensions("Dimensions:")//("Dimensions: "+std::to_string(picture.get_pixbuf()->get_width())+"x"+std::to_string(picture.get_pixbuf()->get_height()))
        , lsize("Size:" +std::to_string(fs::file_size(file))) 
        , ltype("Type" +file.extension().string());

    lname.property_halign() = Gtk::Align::ALIGN_START;
    lpath.property_halign() = Gtk::Align::ALIGN_START;
    ldimensions.property_halign() = Gtk::Align::ALIGN_START;
    lsize.property_halign() = Gtk::Align::ALIGN_START;
    ltype.property_halign() = Gtk::Align::ALIGN_START;

    details_box.pack_start(lname , PACK_SHRINK);
    details_box.pack_start(lpath , PACK_SHRINK);
    details_box.pack_start(ldimensions , PACK_SHRINK);
    details_box.pack_start(lsize , PACK_SHRINK);
    details_box.pack_start(ltype , PACK_SHRINK);

    details_window.add(details_box);
    details_window.show_all_children();

    details_item.signal_clicked().connect([&](){
        details_window.show();    
    });

    //-----------------------------------------------------------------

    //-------------------About Dialog----------------------------------
    AboutDialog aboutd;
    aboutd.set_transient_for(window);

    HeaderBar about_titlebar;
    aboutd.set_titlebar(about_titlebar);
    about_titlebar.set_title("About Image Viewer");

    aboutd.set_program_name("Image Viewer");
    aboutd.set_logo_icon_name("image-viewer");
    aboutd.set_license_type(License::LICENSE_GPL_3_0);

    std::vector<Glib::ustring> authors;
    authors.push_back("Hamza Algohary");
    aboutd.set_authors(authors);

    about_item.signal_clicked().connect([&](){
        int response = aboutd.run();
            aboutd.hide();
    });
    aboutd.show_all_children();
    
    //-----------------------------------------------------------------
    window.show_all_children();

    return app->run(window);

}
