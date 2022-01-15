/*
 * Copyright (c) 2019-2020, Sergey Bugaev <bugaevc@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include "CalculatorWidget.h"
#include <LibCore/System.h>
#include <LibCrypto/NumberTheory/ModularFunctions.h>
#include <LibGUI/Action.h>
#include <LibGUI/Application.h>
#include <LibGUI/Clipboard.h>
#include <LibGUI/Icon.h>
#include <LibGUI/Menu.h>
#include <LibGUI/Menubar.h>
#include <LibGUI/Window.h>
#include <LibGfx/Bitmap.h>
#include <LibMain/Main.h>
#include <stdio.h>
#include <unistd.h>

ErrorOr<int> serenity_main(Main::Arguments arguments)
{
    TRY(Core::System::pledge("stdio recvfd sendfd rpath unix"));
    auto app = TRY(GUI::Application::try_create(arguments));

    TRY(Core::System::pledge("stdio recvfd sendfd rpath"));
    TRY(Core::System::unveil("/res", "r"));
    TRY(Core::System::unveil(nullptr, nullptr));

    auto app_icon = GUI::Icon::default_icon("app-calculator");

    auto window = TRY(GUI::Window::try_create());
    window->set_title("Calculator");
    window->set_resizable(false);
    window->resize(250, 215);

    auto widget = TRY(window->try_set_main_widget<CalculatorWidget>());

    window->set_icon(app_icon.bitmap_for_size(16));

    auto& file_menu = window->add_menu("&File");
    file_menu.add_action(GUI::CommonActions::make_quit_action([](auto&) {
        GUI::Application::the()->quit();
    }));

    auto& edit_menu = window->add_menu("&Edit");
    edit_menu.add_action(GUI::CommonActions::make_copy_action([&](auto&) {
        GUI::Clipboard::the().set_plain_text(widget->get_entry());
    }));
    edit_menu.add_action(GUI::CommonActions::make_paste_action([&](auto&) {
        auto clipboard = GUI::Clipboard::the().fetch_data_and_type();
        if (clipboard.mime_type == "text/plain") {
            if (!clipboard.data.is_empty()) {
                widget->set_entry(Crypto::BigFraction(StringView(clipboard.data)));
            }
        }
    }));

    auto& constants_menu = window->add_menu("&Constants");
    auto const power = Crypto::NumberTheory::Power("10"_bigint, "10"_bigint);

    constants_menu.add_action(GUI::Action::create("&Pi", [&](auto&) {
        widget->set_entry(Crypto::BigFraction { Crypto::SignedBigInteger::create_from(31415926535), power });
    }));
    constants_menu.add_action(GUI::Action::create("&Euler's Constant", [&](auto&) {
        widget->set_entry(Crypto::BigFraction { Crypto::SignedBigInteger::create_from(27182818284), power });
    }));

    auto& round_menu = window->add_menu("&Round");
    round_menu.add_action(GUI::Action::create("&0", [&](auto& action) {
        widget->set_rounding_length(action.text().substring(1).to_uint().value());
    }));

    round_menu.add_action(GUI::Action::create("&2", [&](auto& action) {
        widget->set_rounding_length(action.text().substring(1).to_uint().value());
    }));

    round_menu.add_action(GUI::Action::create("&4", [&](auto& action) {
        widget->set_rounding_length(action.text().substring(1).to_uint().value());
    }));

    auto& help_menu = window->add_menu("&Help");
    help_menu.add_action(GUI::CommonActions::make_about_action("Calculator", app_icon, window));

    window->show();

    return app->exec();
}
