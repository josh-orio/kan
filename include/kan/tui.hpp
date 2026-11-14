#ifndef TUI_HPP
#define TUI_HPP

#include <termui/termui.hpp>

#include <kan/tui/board.hpp>
#include <kan/tui/column.hpp>
#include <kan/tui/comment-list.hpp>
#include <kan/tui/comment.hpp>
#include <kan/tui/commit.hpp>
#include <kan/tui/control-strip.hpp>
#include <kan/tui/description.hpp>
#include <kan/tui/dialog-box.hpp>
#include <kan/tui/editor.hpp>
#include <kan/tui/help-dialog.hpp>
#include <kan/tui/history-viewer.hpp>
// #include <kan/tui/issue-list.hpp>
#include <kan/tui/job-cell.hpp>
#include <kan/tui/job-details.hpp>
#include <kan/tui/job-window-help.hpp>
#include <kan/tui/job-window.hpp>
#include <kan/tui/mini-menu.hpp>
#include <kan/tui/new-comment-window.hpp>
// #include <kan/tui/new-issue-window.hpp>
#include <kan/tui/new-job.hpp>
#include <kan/tui/register-user.hpp>
#include <kan/tui/settings-dialog.hpp>
#include <kan/tui/tab-bar.hpp>
#include <kan/tui/user-menu.hpp>
#include <kan/tui/view-comment-window.hpp>
// #include <kan/tui/view-issue-window.hpp>

namespace kan {
// namespace tui {
extern tui::Board &renderer;
// }
} // namespace kan

#endif
