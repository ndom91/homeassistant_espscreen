#pragma once

// The orientation of an ordinary card head. It knows the available height and
// the requested orientation, not the tile's entity or its controls.
namespace card_orientation {

enum class Choice { automatic, horizontal, vertical };

inline bool stacked(Choice choice, int height, int cell_height, bool fits = true) {
  if (choice == Choice::horizontal) return false;
  if (choice == Choice::vertical) return fits;
  return height >= 2 * cell_height;
}

}  // namespace card_orientation
