/*
 * LVGL custom renderer for modern SPE 40x8 LCD snapshots.
 *
 * Copyright (C) 2026 Phil Taylor (M0VSE)
 * SPDX-License-Identifier: GPL-3.0-only
 */

#include "display/modern_lcd_canvas.h"

#include "display/modern_lcd_font.h"

#include <string.h>

static constexpr uint8_t LCD_COLS = 40;
static constexpr uint8_t LCD_ROWS = 8;
static constexpr uint8_t LCD_SCALE_X = 3;
static constexpr uint8_t LCD_SCALE_Y = 3;
static constexpr uint16_t LCD_WIDTH = LCD_COLS * MODERN_LCD_FONT_WIDTH * LCD_SCALE_X;
static constexpr uint16_t LCD_HEIGHT = LCD_ROWS * MODERN_LCD_FONT_HEIGHT * LCD_SCALE_Y;
static constexpr size_t LCD_CELLS_HEX_LEN = LCD_COLS * LCD_ROWS * 2;
static constexpr size_t LCD_ATTRS_HEX_LEN = LCD_COLS * 2;

static lv_obj_t *modern_lcd_obj = nullptr;
static char modern_lcd_cells[LCD_CELLS_HEX_LEN + 1] = "";
static char modern_lcd_attrs[LCD_ATTRS_HEX_LEN + 1] = "";

static constexpr uint16_t LCD_CELL_WIDTH = MODERN_LCD_FONT_WIDTH * LCD_SCALE_X;
static constexpr uint16_t LCD_CELL_HEIGHT = MODERN_LCD_FONT_HEIGHT * LCD_SCALE_Y;
static constexpr uint16_t LCD_PARTIAL_REDRAW_LIMIT = 80;

static uint8_t hex_nibble(char c)
{
  if (c >= '0' && c <= '9') {
    return static_cast<uint8_t>(c - '0');
  }
  if (c >= 'A' && c <= 'F') {
    return static_cast<uint8_t>(c - 'A' + 10);
  }
  if (c >= 'a' && c <= 'f') {
    return static_cast<uint8_t>(c - 'a' + 10);
  }
  return 0;
}

static uint8_t hex_byte(const char *hex, uint16_t index)
{
  if (!hex) {
    return 0;
  }
  const uint16_t offset = index * 2;
  const char hi = hex[offset];
  const char lo = hex[offset + 1];
  if (!hi || !lo) {
    return 0;
  }
  return static_cast<uint8_t>((hex_nibble(hi) << 4) | hex_nibble(lo));
}

static int16_t clamp_i16(int16_t value, int16_t low, int16_t high)
{
  if (value < low) {
    return low;
  }
  if (value > high) {
    return high;
  }
  return value;
}

static bool intersect_area(lv_area_t *out, const lv_area_t *a, const lv_area_t *b)
{
  out->x1 = a->x1 > b->x1 ? a->x1 : b->x1;
  out->y1 = a->y1 > b->y1 ? a->y1 : b->y1;
  out->x2 = a->x2 < b->x2 ? a->x2 : b->x2;
  out->y2 = a->y2 < b->y2 ? a->y2 : b->y2;
  return out->x1 <= out->x2 && out->y1 <= out->y2;
}

static void draw_rect(lv_layer_t *layer, lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2, lv_color_t color)
{
  lv_area_t area{x1, y1, x2, y2};
  if (!intersect_area(&area, &area, &layer->_clip_area)) {
    return;
  }

  lv_draw_rect_dsc_t dsc;
  lv_draw_rect_dsc_init(&dsc);
  dsc.bg_opa = LV_OPA_COVER;
  dsc.bg_color = color;
  dsc.border_opa = LV_OPA_TRANSP;
  dsc.outline_opa = LV_OPA_TRANSP;
  dsc.shadow_opa = LV_OPA_TRANSP;
  lv_draw_rect(layer, &dsc, &area);
}

static void modern_lcd_draw_event(lv_event_t *event)
{
  lv_layer_t *layer = lv_event_get_layer(event);
  if (!layer || !modern_lcd_obj) {
    return;
  }

  lv_area_t coords;
  lv_obj_get_coords(modern_lcd_obj, &coords);
  lv_area_t draw_area;
  if (!intersect_area(&draw_area, &coords, &layer->_clip_area)) {
    return;
  }

  const lv_color_t bg = lv_color_hex(0x00B3FE);
  const lv_color_t fg = lv_color_hex(0x050505);
  draw_rect(layer, draw_area.x1, draw_area.y1, draw_area.x2, draw_area.y2, bg);

  const int16_t first_row = clamp_i16((draw_area.y1 - coords.y1) / (MODERN_LCD_FONT_HEIGHT * LCD_SCALE_Y), 0, LCD_ROWS - 1);
  const int16_t last_row = clamp_i16((draw_area.y2 - coords.y1) / (MODERN_LCD_FONT_HEIGHT * LCD_SCALE_Y), 0, LCD_ROWS - 1);
  const int16_t first_col = clamp_i16((draw_area.x1 - coords.x1) / (MODERN_LCD_FONT_WIDTH * LCD_SCALE_X), 0, LCD_COLS - 1);
  const int16_t last_col = clamp_i16((draw_area.x2 - coords.x1) / (MODERN_LCD_FONT_WIDTH * LCD_SCALE_X), 0, LCD_COLS - 1);

  for (uint8_t row = static_cast<uint8_t>(first_row); row <= last_row; ++row) {
    for (uint8_t col = static_cast<uint8_t>(first_col); col <= last_col; ++col) {
      uint8_t code = hex_byte(modern_lcd_cells, static_cast<uint16_t>(row) * LCD_COLS + col);
      const uint8_t attr = (hex_byte(modern_lcd_attrs, col) & (1U << row)) ? 1 : 0;
      if (attr & 0x80) {
        code = static_cast<uint8_t>(code - 32);
      }

      for (uint8_t gy = 0; gy < MODERN_LCD_FONT_HEIGHT; ++gy) {
        uint8_t scan = modern_lcd_font_row(code, gy);
        if (attr) {
          scan = static_cast<uint8_t>(~scan);
        }

        uint8_t gx = 0;
        while (gx < MODERN_LCD_FONT_WIDTH) {
          while (gx < MODERN_LCD_FONT_WIDTH && !(scan & (1U << (MODERN_LCD_FONT_WIDTH - 1 - gx)))) {
            ++gx;
          }
          const uint8_t run_start = gx;
          while (gx < MODERN_LCD_FONT_WIDTH && (scan & (1U << (MODERN_LCD_FONT_WIDTH - 1 - gx)))) {
            ++gx;
          }
          if (run_start == gx) {
            continue;
          }

          const lv_coord_t x1 = coords.x1 + (static_cast<uint16_t>(col) * MODERN_LCD_FONT_WIDTH + run_start) * LCD_SCALE_X;
          const lv_coord_t y1 = coords.y1 + (static_cast<uint16_t>(row) * MODERN_LCD_FONT_HEIGHT + gy) * LCD_SCALE_Y;
          const lv_coord_t x2 = coords.x1 + (static_cast<uint16_t>(col) * MODERN_LCD_FONT_WIDTH + gx) * LCD_SCALE_X - 1;
          const lv_coord_t y2 = y1 + LCD_SCALE_Y - 1;
          draw_rect(layer, x1, y1, x2, y2, fg);
        }
      }
    }
  }
}

void modern_lcd_canvas_create(lv_obj_t *parent)
{
  if (modern_lcd_obj || !parent) {
    return;
  }

  modern_lcd_obj = lv_obj_create(parent);
  lv_obj_remove_style_all(modern_lcd_obj);
  lv_obj_set_size(modern_lcd_obj, LCD_WIDTH, LCD_HEIGHT);
  lv_obj_set_x(modern_lcd_obj, 21);
  lv_obj_set_y(modern_lcd_obj, 20);
  lv_obj_add_flag(modern_lcd_obj, LV_OBJ_FLAG_HIDDEN);
  lv_obj_remove_flag(modern_lcd_obj, LV_OBJ_FLAG_CLICKABLE);
  lv_obj_remove_flag(modern_lcd_obj, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_add_event_cb(modern_lcd_obj, modern_lcd_draw_event, LV_EVENT_DRAW_MAIN, nullptr);
}

void modern_lcd_canvas_hide()
{
  if (modern_lcd_obj) {
    lv_obj_add_flag(modern_lcd_obj, LV_OBJ_FLAG_HIDDEN);
  }
}

void modern_lcd_canvas_show()
{
  if (modern_lcd_obj) {
    lv_obj_remove_flag(modern_lcd_obj, LV_OBJ_FLAG_HIDDEN);
  }
}

static void invalidate_cell(uint8_t row, uint8_t col)
{
  if (!modern_lcd_obj) {
    return;
  }

  lv_area_t coords;
  lv_obj_get_coords(modern_lcd_obj, &coords);
  lv_area_t area{
    static_cast<lv_coord_t>(coords.x1 + col * LCD_CELL_WIDTH),
    static_cast<lv_coord_t>(coords.y1 + row * LCD_CELL_HEIGHT),
    static_cast<lv_coord_t>(coords.x1 + (col + 1) * LCD_CELL_WIDTH - 1),
    static_cast<lv_coord_t>(coords.y1 + (row + 1) * LCD_CELL_HEIGHT - 1)
  };
  lv_obj_invalidate_area(modern_lcd_obj, &area);
}

bool modern_lcd_canvas_render(const char *lcd_cells_hex, const char *lcd_attrs_hex)
{
  if (!modern_lcd_obj || !lcd_cells_hex || !lcd_cells_hex[0]) {
    return false;
  }

  const char *attrs = lcd_attrs_hex ? lcd_attrs_hex : "";
  if (strcmp(modern_lcd_cells, lcd_cells_hex) == 0 && strcmp(modern_lcd_attrs, attrs) == 0) {
    modern_lcd_canvas_show();
    return true;
  }

  bool changed_cells[LCD_ROWS][LCD_COLS] = {};
  uint16_t changed_count = 0;
  for (uint8_t row = 0; row < LCD_ROWS; ++row) {
    for (uint8_t col = 0; col < LCD_COLS; ++col) {
      const uint16_t cell_index = static_cast<uint16_t>(row) * LCD_COLS + col;
      const uint8_t old_code = hex_byte(modern_lcd_cells, cell_index);
      const uint8_t new_code = hex_byte(lcd_cells_hex, cell_index);
      const bool old_attr = (hex_byte(modern_lcd_attrs, col) & (1U << row)) != 0;
      const bool new_attr = (hex_byte(attrs, col) & (1U << row)) != 0;
      if (old_code != new_code || old_attr != new_attr) {
        changed_cells[row][col] = true;
        ++changed_count;
      }
    }
  }

  strncpy(modern_lcd_cells, lcd_cells_hex, sizeof(modern_lcd_cells) - 1);
  modern_lcd_cells[sizeof(modern_lcd_cells) - 1] = '\0';
  strncpy(modern_lcd_attrs, attrs, sizeof(modern_lcd_attrs) - 1);
  modern_lcd_attrs[sizeof(modern_lcd_attrs) - 1] = '\0';

  modern_lcd_canvas_show();
  if (changed_count == 0 || changed_count > LCD_PARTIAL_REDRAW_LIMIT) {
    lv_obj_invalidate(modern_lcd_obj);
  } else {
    for (uint8_t row = 0; row < LCD_ROWS; ++row) {
      for (uint8_t col = 0; col < LCD_COLS; ++col) {
        if (changed_cells[row][col]) {
          invalidate_cell(row, col);
        }
      }
    }
  }
  return true;
}
