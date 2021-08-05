/**
 *  This file is part of libnao-ui.
 *
 *  libnao-ui is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  libnao-ui is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with libnao-ui.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "directional_layout.h"

#include <libnao/util/ranges.h>
#include <libnao/util/formatters.h>

nao::ui::directional_layout::directional_layout(window& parent, layout_direction dir)
    : layout{ parent }
    , _direction{ dir } {
    
}

void nao::ui::directional_layout::reposition() {
    int count = static_cast<int>(children.size());
    logger().trace("Repositioning {} children to fit in {}", count, client_size());

    if (count == 0) {
        return;
    }

    auto [w, h] = client_size();
    auto [top, right, bot, left] = content_margins();
    long spacing = content_spacing();
    long max_h = h - top - bot;
    long max_w = w - left - right;

    HDWP dwp = BeginDeferWindowPos(count);

    switch (_direction) {
        case layout_direction::horizontal: {
            // Disregard margins
            long remaining_pixels = std::max<long>(0, w - left - right - ((count - 1) * spacing));

            struct layout_helper {
                window& win;
                size max;
                long width;
            };

            auto converter = [&](std::unique_ptr<layout_item>& ptr) {
                window& win = ptr->item();
                return layout_helper{
                    .win = win,
                    .max = win.maximum_size().fit_in(
                        win.parent() ? win.parent()->client_size() : size::max()),
                    .width = 0,
                };
            };

            std::vector<layout_helper> sizes = children | std::views::transform(converter) | to_vector;

            // Increase each element's size by 1 pixel until the control is at it's max size,
            // or there is no more space left. This should provide working fill-to-fit,
            // and the performance was benchmarked to not be signficantly slower than the actual
            // DeferWindowPos step (~0.7ms vs 3.5ms).
            auto it = sizes.begin();
            while (remaining_pixels > 0) {
                auto& [win, max, width] = *it++;

                if (width < max.w) {
                    width += 1;
                    remaining_pixels -= 1;
                }

                if (it == sizes.end()) {
                    it = sizes.begin();
                }
            }


            for (long pos_x = left; const auto & [win, max, width] : sizes) {
                margins padding = win.padding();

                dwp = DeferWindowPos(dwp, win.handle(), nullptr,
                    pos_x + padding.left,
                    top + padding.top,
                    width - padding.left - padding.right,
                    std::min<long>(max.h, max_h) - padding.top - padding.bot, 0);

                pos_x += width + spacing;
            }

            break;
        }

        case layout_direction::vertical: {
            // Disregard margins
            long remaining_pixels = std::max<long>(0, h - top - bot - ((count - 1) * spacing));

            struct layout_helper {
                window& win;
                size max;
                long height;
            };

            auto converter = [&](std::unique_ptr<layout_item>& ptr) {
                window& win = ptr->item();
                return layout_helper{
                    .win = win,
                    .max = win.maximum_size().fit_in(
                        win.parent() ? win.parent()->client_size() : size::max()),
                    .height = 0,
                };
            };

            std::vector<layout_helper> sizes = children | std::views::transform(converter) | to_vector;

            auto it = sizes.begin();
            while (remaining_pixels > 0) {
                auto& [win, max, height] = *it++;

                if (height < max.h) {
                    height += 1;
                    remaining_pixels -= 1;
                }

                if (it == sizes.end()) {
                    it = sizes.begin();
                }
            }


            for (long pos_y = top; const auto & [win, max, height] : sizes) {
                margins padding = win.padding();

                dwp = DeferWindowPos(dwp, win.handle(), nullptr,
                    left + padding.left,
                    pos_y + padding.top,
                    std::min<long>(max.w, max_w) - padding.left - padding.right,
                    height - padding.top - padding.bot, 0);

                pos_y += height + spacing;
            }

            if (!EndDeferWindowPos(dwp)) {
                logger().critical("Failed to reposition {} children", count);
            }

            break;
        }
    }

    if (!EndDeferWindowPos(dwp)) {
        logger().critical("Failed to reposition {} children", count);
    }
}

nao::ui::horizontal_layout::horizontal_layout(window& parent)
    : directional_layout{ parent, layout_direction::horizontal } {

}

nao::ui::vertical_layout::vertical_layout(window& parent)
    : directional_layout{ parent, layout_direction::vertical } {
    
}
