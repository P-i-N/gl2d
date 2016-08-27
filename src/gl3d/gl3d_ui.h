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
struct control
{
  bool used = false;
  ibox2 rect = ibox2();

  virtual void reset() { used = false; }
};

//---------------------------------------------------------------------------------------------------------------------
struct ctrl_child_area: control
{
  std::unordered_map<hash_t, std::unique_ptr<control>> controls;

  void reset() override
  {
    control::reset();
    for (auto &&kvp : controls)
      kvp.second->reset();
  }
};

//---------------------------------------------------------------------------------------------------------------------
struct ctrl_window: ctrl_child_area
{
  
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

  g_current_ctx->root.reset();
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
  g_current_ctx = nullptr;
}

//---------------------------------------------------------------------------------------------------------------------
bool window(const std::string &text)
{
  return true;
}
  
//---------------------------------------------------------------------------------------------------------------------
void label(const std::string &text)
{

}

//---------------------------------------------------------------------------------------------------------------------
bool button(const std::string &text)
{
  return false;
}

} }

#endif // __GL3D_UI_H_IMPL__
#endif // GL3D_IMPLEMENTATION
