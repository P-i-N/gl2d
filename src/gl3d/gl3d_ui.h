#ifndef __GL3D_UI_H__
#define __GL3D_UI_H__

#include "gl3d_win32.h"

namespace gl3d { namespace ui {

void begin(size_t id, int width, int height);

void begin();

void end();

bool window(const std::string &text);

void label(const std::string &text);

bool button(const std::string &text);

} }

#endif // __GL3D_UI_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
#ifndef __GL3D_UI_H_IMPL__
#define __GL3D_UI_H_IMPL__

#include <string>

namespace gl3d { namespace ui {

//---------------------------------------------------------------------------------------------------------------------
struct control_key
{
  union
  {
    struct
    {
      uint32_t type: 8;
      uint32_t index: 24;
      uint32_t hash;
    };

    uint64_t value;
  };

  bool operator==(const control_key &rhs) const { return value == rhs.value; }
  bool operator!=(const control_key &rhs) const { return value != rhs.value; }
  bool operator<(const control_key &rhs) const { return value < rhs.value; }
};

} }

namespace std { template <> struct hash<gl3d::ui::control_key> {
  size_t operator()(const gl3d::ui::control_key &key) const { return hash<uint64_t>()(key.value); }
}; }

namespace gl3d { namespace ui {

//---------------------------------------------------------------------------------------------------------------------
struct control
{
  enum class type
  {
    child_area, label, button, window
  };
  
  virtual type control_type() const = 0;

  bool used = false;
  ibox2 rect = ibox2();

  virtual void reset() { used = false; }
};

//---------------------------------------------------------------------------------------------------------------------
struct ctrl_child_area: control
{
  static const type class_type = type::child_area;
  type control_type() const override { return class_type; }

  std::unordered_map<control_key, std::unique_ptr<control>> controls;

  void reset() override
  {
    for (auto iter = controls.begin(); iter != controls.end(); )
      if (!iter->second->used)
        iter = controls.erase(iter);
      else
        ++iter;

    control::reset();
    for (auto &&kvp : controls)
      kvp.second->reset();
  }
};

//---------------------------------------------------------------------------------------------------------------------
struct ctrl_window: ctrl_child_area
{
  static const type class_type = type::window;
  type control_type() const override { return class_type; }
};

//---------------------------------------------------------------------------------------------------------------------
struct ctrl_label: control
{
  static const type class_type = type::label;
  type control_type() const override { return class_type; }
};

//---------------------------------------------------------------------------------------------------------------------
struct context
{
  std::vector<ctrl_window *> windows;
  std::vector<ctrl_child_area *> area_stack;
  ctrl_child_area root;
  char id_string_buffer[65536];
  char *id_string_cursor = id_string_buffer;
  std::vector<char *> id_string_stack;
  std::vector<hash_t> id_hash_stack = { 0 };
  
  void reset()
  {
    root.reset();
    area_stack.clear();
    area_stack.push_back(&root);
  }

  hash_t id() const { return id_hash_stack.back(); }

  hash_t id(const char *str, size_t len = 0) const
  {
    if (!len) len = strlen(str);
    assert(id_string_cursor + len < id_string_buffer + 65536);
    memcpy(id_string_cursor, str, len);
    return detail::hash(id_string_buffer, id_string_cursor - id_string_buffer + len, detail::fnv_offset);
  }

  hash_t push_id(const char *str, size_t len = 0)
  {
    if (!len) len = strlen(str);
    assert(id_string_cursor + len < id_string_buffer + 65536);
    memcpy(id_string_cursor, str, len);

    id_string_stack.push_back(id_string_cursor);
    id_string_cursor += len;
    id_hash_stack.push_back(detail::hash(id_string_buffer, id_string_cursor - id_string_buffer, detail::fnv_offset));
    return id_hash_stack.back();
  }

  hash_t push_id(hash_t id)
  {
    char buff[32];
    _ltoa(id, buff, 10);
    return push_id(buff);
  }

  void pop_id()
  {
    assert(!id_string_stack.empty());

    id_string_cursor = id_string_stack.back();
    id_string_stack.pop_back();
    id_hash_stack.pop_back();
  }

  template <typename T>
  T *get_or_create(const std::string &text, size_t index = 0)
  {
    control_key key;
    key.type = static_cast<uint32_t>(T::class_type);
    key.index = static_cast<uint32_t>(index);
    key.hash = id(text.c_str(), text.size());

    auto &controls = area_stack.back()->controls;
    auto iter = controls.find(key);
    if (iter != controls.end())
    {
      iter->second->used = true;
      return static_cast<T *>(iter->second.get());
    }

    T *result = new T();
    result->used = true;
    controls[key] = std::unique_ptr<control>(result);
    return result;
  }
};

std::unordered_map<size_t, std::unique_ptr<context>> g_contexts;
context *g_current_ctx = nullptr;
bool g_is_hooked = false;

//---------------------------------------------------------------------------------------------------------------------
void on_event_handler(event &e)
{
  
}

//---------------------------------------------------------------------------------------------------------------------
void begin(const size_t id, int width, int height)
{
  assert(g_current_ctx == nullptr);

  if (!g_is_hooked)
  {
    on_event += std::function<decltype(on_event_handler)>(on_event_handler);
    g_is_hooked = true;
  }

  auto iter = g_contexts.find(id);
  if (iter == g_contexts.end())
  {
    g_current_ctx = new context();
    g_contexts[id] = std::unique_ptr<context>(g_current_ctx);
  }
  else
    g_current_ctx = iter->second.get();

  g_current_ctx->reset();
}

//---------------------------------------------------------------------------------------------------------------------
void begin()
{
  auto wid = current.current_window_id;
  auto size = get_window_size(wid);
  begin(static_cast<size_t>(wid), size.x, size.y);
}

//---------------------------------------------------------------------------------------------------------------------
void end()
{
  assert(g_current_ctx);

  g_current_ctx->area_stack.pop_back();

  if (g_current_ctx->area_stack.empty())
  {
    g_current_ctx = nullptr;
  }
}

//---------------------------------------------------------------------------------------------------------------------
bool window(const std::string &text)
{
  return true;
}
  
//---------------------------------------------------------------------------------------------------------------------
void label(const std::string &text)
{
  auto ctrl = g_current_ctx->get_or_create<ctrl_label>(text);
}

//---------------------------------------------------------------------------------------------------------------------
bool button(const std::string &text)
{
  return false;
}

} }

#endif // __GL3D_UI_H_IMPL__
#endif // GL3D_IMPLEMENTATION
