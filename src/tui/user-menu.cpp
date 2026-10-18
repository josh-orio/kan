#include <kan/tui.hpp>

namespace kan {
namespace tui {

UserMenu::UserMenu() : _users(board.Users()), _cursor(0), _start_line(0){}

void UserMenu::CursorUp(unsigned int count) {
    internal_update();
    _cursor     = (_cursor > 0) ? std::max(0, int(_cursor - count)) : 0; // decrement but dont let (cursor < 0)
    _start_line = (_cursor < _start_line) ? _cursor : _start_line;       // don't let cursor go 'above' start line
}

void UserMenu::CursorDown(unsigned int count) {
    internal_update();
    _cursor     = (_cursor < _users.size() - count) ? std::min(_users.size() - 1, static_cast<size_t>(_cursor + count))
                                                    : _users.size() - 1;                              // increment but dont let (cursor > elements.size)
    _start_line = (_cursor >= _start_line + _visible_lines) ? _cursor - _visible_lines + 1 : _start_line; // don't let cursor go 'below' bottom of visible lines
}

unsigned int UserMenu::getCursor() { return _cursor; }

UserMenu &UserMenu::setCursor(unsigned int n) {
    _cursor = n;
    return *this;
}

void UserMenu::display() {
    internal_update();

    auto terminal_half_w = termui::terminal.halfWidth();
    auto terminal_half_h = termui::terminal.halfHeight();

    auto window_start_row = terminal_half_h - (_height / 2);
    auto window_start_col = terminal_half_w - (_width / 2);

    // box
    termui::terminal.MoveCursor(window_start_row, window_start_col);
    termui::Box(_width, _height, termui::Borders::rounded, termui::Styles::faint).render();

    // title header
    termui::terminal.MoveCursor(window_start_row + 1, window_start_col + 2);
    termui::Text(_title, termui::Styles::bold).width(_width - 4).height(1).render();

    // list items
    for (size_t i = _start_line; i < std::min(_users.size(), static_cast<size_t>(_start_line + _visible_lines)); i++) {
        int line_pos = window_start_row + 3 + (i - _start_line) * (_line_height + _line_spacing);

        if (i == _cursor) {
            termui::terminal.MoveCursor(line_pos, window_start_col + 2);
            termui::terminal.write("·");
        }

        termui::terminal.MoveCursor(line_pos, window_start_col + 4);
        termui::Text(_users.at(i).Name()).size(_width - 6, 1).render();
    }

    // control strip
    termui::terminal.MoveCursor(window_start_row + _height - 2, window_start_col + 2);
    kan::tui::ControlStrip({
                                {"esc", "close"},
                                {"↑↓", "navigate"},
                                {"↵", "select"},
                            })
        .width(_width - 4)
        .render();

    termui::terminal.flush();
}

void UserMenu::process_input() {
    auto inp = termui::terminal.read();

    if (inp == termui::key::ESC) {
        state = termui::Interface::State::Exit;
    }
    else if (inp == termui::key::U_ARROW) {
        CursorUp();
    }
    else if (inp == termui::key::D_ARROW) {
        CursorDown();
    }
    else if (inp == termui::key::ENTER) {
        state = termui::Interface::State::Submit;
    }
}

std::pair<termui::Interface::State, User> UserMenu::show() {
    state = State::Continue;

    do {
        display();
        process_input();

    } while (state == State::Continue);

    return {state, _users.at(_cursor)};
}

void UserMenu::internal_update() {
    _visible_lines = 0;
    while (true) {
        // calculates lines used by displaying another row of cells
        int space_used = (_line_height * (_visible_lines + 1)) + (_line_spacing * _visible_lines);

        if (space_used > _height - 6) { // -6 for other stuff
            break;
        }
        _visible_lines++;
    }
}

} // namespace tui
} // namespace kan