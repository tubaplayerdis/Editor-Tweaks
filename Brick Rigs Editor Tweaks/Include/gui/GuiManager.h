#pragma once
#include <memory>
#include <atomic>

#define DECLARE_GUI_MENU(name) extern gui_menu name;
#define DEFINE_GUI_MENU(name, lamb, ...) gui_menu name = gui_menu(lamb, __VA_ARGS__);

struct gui_menu;

class gui_manager
{
    friend class std::unique_ptr<gui_manager>;

    gui_manager();

public:

    static gui_manager* get();
    static void shutdown();

    void hide_all();
    bool are_all_hidden();
    void display_all_previous();

    void add_menu(gui_menu* menu);
    void remove_menu(gui_menu* menu);

    //Thread safe function to change menu visibility. these are not actually thread safe but work way better.
    void set_menu_visibility(gui_menu* menu, bool visibility);
    void toggle_menu_visibility(gui_menu* menu);
};

struct gui_menu
{
    void(*menu)();
    void(*custom_toggle)(bool);
    std::atomic<bool> is_visible;

    gui_menu(void(*in_menu)(), void(*in_custom_toggle)(bool) = nullptr)
    {
        menu = in_menu;
        is_visible = false;
        custom_toggle = in_custom_toggle;
    }

    void toggle()
    {
        gui_manager::get()->toggle_menu_visibility(this);
    }

    void display()
    {
        gui_manager::get()->set_menu_visibility(this, true);
    }

    void hide()
    {
        gui_manager::get()->set_menu_visibility(this, false);
    }
};