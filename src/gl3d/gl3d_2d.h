#ifndef __GL2D_H__
#define __GL2D_H__

#include "gl3d.h"

namespace gl3d {

namespace detail {
  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *vertex_shader_code2d = R"GLSHADER(
layout(location = 0) in vec2 vert_Position;
layout(location = 1) in vec4 vert_Color;
layout(location = 2) in vec2 vert_UV;

uniform vec2 u_ScreenSize;

out vec4 Color;
out vec2 UV;

void main()
{
  vec2 clipPos = ((vert_Position + vec2(0.375)) / u_ScreenSize);
  clipPos.y = 1.0 - clipPos.y;
  clipPos = clipPos * 2.0 - 1.0;

  gl_Position = vec4(clipPos, 0, 1);
  Color = vert_Color;
  UV = vert_UV;
}
)GLSHADER";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *fragment_shader_code2d = R"GLSHADER(
uniform sampler2D u_FontTexture;

in vec4 Color;
in vec2 UV;

out vec4 out_Color;

void main()
{
  out_Color = texture(u_FontTexture, UV) * Color;
}
)GLSHADER";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *default_font_base64 =
  "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"
  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA==";

//---------------------------------------------------------------------------------------------------------------------
static bool is_base64(uint8_t c) { return (isalnum(c) || (c == '+') || (c == '/')); }

//---------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> base64_decode(const char *encoded_string)
{
  static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  size_t len = strlen(encoded_string);
  int i = 0, cursor = 0;
  uint8_t char_array_4[4], char_array_3[3];
  std::vector<uint8_t> result;

  while (len-- && (encoded_string[cursor] != '=') && is_base64(encoded_string[cursor]))
  {
    char_array_4[i++] = encoded_string[cursor++];

    if (i == 4)
    {
      for (i = 0; i < 4; ++i) char_array_4[i] = static_cast<uint8_t>(base64_chars.find(char_array_4[i]));

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); ++i) result.push_back(char_array_3[i]);

      i = 0;
    }
  }

  if (i)
  {
    for (int j = i; j < 4; j++) char_array_4[j] = 0;
    for (int j = 0; j < 4; j++) char_array_4[j] = static_cast<uint8_t>(base64_chars.find(char_array_4[j]));

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (int j = 0; (j < i - 1); j++) result.push_back(char_array_3[j]);
  }

  return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct font : public detail::ref_counted
{
  typedef detail::ptr<font> ptr;

  struct char_info
  {
    int id = 0;
    ivec2 size;
    ibox2 box;
    ivec2 offset;
    vec2 uv[4];
    int x_advance;
  };

  int base;
  int line_height;
  std::map<int, char_info> char_infos;
  std::map<uint64_t, int> kernings;
  texture::ptr font_texture = new texture();

#define GL3D_DATA_EXTRACT(_Type) \
  (*reinterpret_cast<const _Type *>(data)); data = reinterpret_cast<const uint8_t *>(data) + sizeof(_Type)

  font(const void *data, size_t length)
  {
    int texWidth = GL3D_DATA_EXTRACT(short);
    int texHeight = GL3D_DATA_EXTRACT(short);
    float texWidthf = static_cast<float>(texWidth);
    float texHeightf = static_cast<float>(texHeight);

    uint32_t *image = new uint32_t[texWidth * texHeight];
    memset(image, 0, texWidth * texHeight * sizeof(uint32_t));
      
    uint32_t *cursorOutput = image;
    auto cursorInput = reinterpret_cast<const uint8_t *>(data);

    for (int xy = 0; xy < texWidth * texHeight; xy += 8, ++cursorInput)
    {
      auto byte = *cursorInput;
      for (int i = 0; i < 8; ++i, ++cursorOutput, byte >>= 1)
        if (byte & 1)
        {
          *cursorOutput = 0xFFFFFFFFu;
          *(cursorOutput + texWidth + 1) = 0xFF000000u; // Remove this to get rid of black font shadow
        }
    }
      
    image[texWidth * texHeight - 1] = 0xFFFFFFFFu;

    font_texture->set_params(texWidth, texHeight, GL_RGBA, 1, 1);
    font_texture->alloc_pixels(image);
    font_texture->set_wrap(gl.CLAMP_TO_EDGE);

    delete [] image;
    data = cursorInput;

    line_height = GL3D_DATA_EXTRACT(uint8_t);
    base = GL3D_DATA_EXTRACT(uint8_t);

    int numChars = GL3D_DATA_EXTRACT(int);
    for (int i = 0; i < numChars; ++i)
    {
      char_info chi;
      chi.id = GL3D_DATA_EXTRACT(int);
      chi.box.min.x = GL3D_DATA_EXTRACT(uint16_t);
      chi.box.min.y = GL3D_DATA_EXTRACT(uint16_t);
      chi.size.x = 1 + GL3D_DATA_EXTRACT(uint8_t);
      chi.size.y = 1 + GL3D_DATA_EXTRACT(uint8_t);
      chi.box.max.x = chi.box.min.x + chi.size.x;
      chi.box.max.y = chi.box.min.y + chi.size.y;
      chi.offset.x = GL3D_DATA_EXTRACT(int8_t);
      chi.offset.y = GL3D_DATA_EXTRACT(int8_t);
      chi.x_advance = GL3D_DATA_EXTRACT(int8_t);
      for (int j = 0; j < 4; ++j)
        chi.uv[j] = vec2(chi.box.corner(j).x / texWidthf, chi.box.corner(j).y / texHeightf);

      char_infos[chi.id] = chi;
    }

    int numKernings = GL3D_DATA_EXTRACT(int);
    for (int i = 0; i < numKernings; ++i)
    {
      uint64_t first = static_cast<uint64_t> GL3D_DATA_EXTRACT(int);
      uint64_t second = static_cast<uint64_t> GL3D_DATA_EXTRACT(int);
      kernings[first | (second << 32ull)] = GL3D_DATA_EXTRACT(int8_t);
    }
  }

#undef GL3D_DATA_EXTRACT

  font(const std::vector<uint8_t> &bytes): font(bytes.data(), bytes.size()) { }
  font(const char *base64Data): font(base64_decode(base64Data)) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vertex2d : layout<vec2, vec4, vec2>
{
  vec2 pos;
  vec4 color;
  vec2 uv;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct draw_call
{
  bool triangles; // true for triangles, false for lines
  size_t length;  // number of vertices

  draw_call(bool tris, size_t len): triangles(tris), length(len) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct state
{
  ibox2 viewport;
  ibox2 scissors;
  vec4 color;
  texture::ptr texture;
  font::ptr font;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context2d
{
public:
  context2d()
  {

  }

  virtual ~context2d()
  {
    done();
  }
  
  bool initialized() const { return _initialized; }

  bool init();

  void done();

  void clear()
  {
    _context3d.clear();
    _geometry->clear();
    _drawCalls.clear();
    _drawCalls.emplace_back(true, 0);
    _state.color = vec4::one();
  }

  void color(const vec4 &c) { _state.color = c; }

  void color(float r, float g, float b, float a = 1.0f) { _state.color = vec4(r, g, b, a); }

  void color(uint32_t argb) { _state.color = vec4(argb); }

  const vec4 &color() const { return _state.color; }

  void line(const vec2 &a, const vec2 &b)
  {
    if (!_drawCalls.back().triangles)
      _drawCalls.back().length += 2;
    else
      _drawCalls.emplace_back(false, 2);
    
    auto *v = _geometry->alloc_vertices(2);
    v->pos = a;
    v->color = _state.color;
    v->uv = vec2(1, 1);
    ++v;
    v->pos = b;
    v->color = _state.color;
    v->uv = vec2(1, 1);
  }

  void line(float x1, float y1, float x2, float y2) { line(vec2(x1, y1), vec2(x2, y2)); }

  void linei(int x1, int y1, int x2, int y2) { line(vec2(x1, y1), vec2(x2, y2)); }

  void rectangle(const vec2 &a, const vec2 &b, bool filled = false)
  {
    if (filled)
    {
      if (_drawCalls.back().triangles)
        _drawCalls.back().length += 6;
      else
        _drawCalls.emplace_back(true, 6);
      
      auto *v = _geometry->alloc_vertices(6);
      v->pos = a;
      v->color = _state.color;
      v->uv = vec2(1, 1);
      ++v;
      v->pos = vec2(b.x, a.y);
      v->color = _state.color;
      v->uv = vec2(1, 1);
      ++v;
      v->pos = vec2(a.x, b.y);
      v->color = _state.color;
      v->uv = vec2(1, 1);
      
      v[1] = *v;
      v[2] = v[-1];
      v += 3;

      v->pos = vec2(b.x, b.y);
      v->color = _state.color;
      v->uv = vec2(1, 1);
    }
    else
    {
      vec2 c(b.x - 1.0f, b.y - 1.0f);
      line(a.x, a.y, c.x, a.y);
      line(c.x, a.y, c.x, c.y);
      line(c.x, c.y, a.x, c.y);
      line(a.x, c.y, a.x, a.y);
    }
  }

  void rectangle(float x1, float y1, float x2, float y2, bool filled = false) { rectangle(vec2(x1, y1), vec2(x2, y2), filled); }

  void rectanglei(int x1, int y1, int x2, int y2, bool filled = false)  { rectangle(vec2(x1, y1), vec2(x2, y2), filled); }
  
  void text(const vec2 &pos, const char *fmt, va_list &ap)
  {
    if (!fmt || !(*fmt)) return;

    char buff[1024];
    size_t length = static_cast<size_t>(vsnprintf(buff, 1024, fmt, ap));
    if (!length) return;

    float x = pos.x;
    float y = pos.y;
    const char *text = buff;
    const char *end = text + length;
    vec4 color = _state.color;

    while (text < end)
    {
      const char *mark = strchr(text, '^');
    
      if (!mark)
        mark = end;

      int partLen = static_cast<int>(mark - text);

      if (partLen)
        print_substring(x, y, color, text, partLen);

      if (mark[0] == '^')
      {
        static const char *colorString = "0123456789abcdef";
        const char *colorChar = strchr(colorString, tolower(mark[1]));
      
        if (colorChar)
        {
          const float q = 0.25f;
          const float h = 0.5f;

          switch (*colorChar)
          {
            case '0': color = vec4(0xFF000000); break;
            case '1': color = vec4(0xFF000080); break;
            case '2': color = vec4(0xFF008000); break;
            case '3': color = vec4(0xFF008080); break;
            case '4': color = vec4(0xFF800000); break;
            case '5': color = vec4(0xFF800080); break;
            case '6': color = vec4(0xFF808000); break;
            case '7': color = vec4(0xFF404040); break;
            case '8': color = vec4(0xFF808080); break;
            case '9': color = vec4(0xFF4080FF); break;
            case 'a': color = vec4(0xFF40FF40); break;
            case 'b': color = vec4(0xFF40FFFF); break;
            case 'c': color = vec4(0xFFFF8040); break;
            case 'd': color = vec4(0xFFFF40FF); break;
            case 'e': color = vec4(0xFFFFFF40); break;

            default:
            case 'f': color = vec4(0xFFFFFFFF); break;
          }
        }
        else
          ++partLen;
      }

      text = mark + 2;
    }
  }

  void text(float x, float y, const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    text(vec2(x, y), fmt, ap);
    va_end(ap);
  }

  void texti(int x, int y, const char *fmt, ...)
  {
    va_list ap;
    va_start(ap, fmt);
    text(vec2(x, y), fmt, ap);
    va_end(ap);
  }

  void render(int x, int y, int width, int height)
  {
    using namespace detail;

    _context3d.clear();

    glViewport(x, y, width, height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    _context3d.bind(_geometry);
    _context3d.bind(_technique);
    _context3d.set_uniform("u_ScreenSize", vec2(width, height));
    _context3d.set_uniform("u_FontTexture", _state.font->font_texture);

    size_t startVertex = 0;
    for (auto &&dc : _drawCalls)
    {
      if (!dc.length) continue;

      glDrawArrays(dc.triangles ? GL_TRIANGLES : GL_LINES, static_cast<GLint>(startVertex), static_cast<GLsizei>(dc.length));
      startVertex += dc.length;
    }

    clear(); 
  }

  void render(int width, int height) { render(0, 0, width, height); }

private:
  void print_substring(float &x, float &y, const vec4 &color, const char *text, size_t length);

  bool _initialized = false;
  detail::state _state;
  context3d _context3d;
  detail::ptr<technique> _technique = new technique();
  detail::ptr<custom_geometry<detail::vertex2d>> _geometry = new custom_geometry<detail::vertex2d>();
  std::vector<detail::draw_call> _drawCalls;
};

extern detail::font *default_font;
extern detail::font *monospace_font;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

}

#endif // __GL2D_H__

#if defined(GL3D_IMPLEMENTATION)
#ifndef __GL3D_2D_H_IMPL__
#define __GL3D_2D_H_IMPL__
namespace gl3d {

static detail::font *default_font = nullptr;
static detail::font *monospace_font = nullptr;

//---------------------------------------------------------------------------------------------------------------------
bool context2d::init()
{
  using namespace detail;

  if (_initialized)
    return true;

  gl.init();

  _technique->set_vert_source(vertex_shader_code2d);
  _technique->set_frag_source(fragment_shader_code2d);

  if (!default_font) default_font = new detail::font(detail::default_font_base64);
  default_font->ref();
  
  _state.font = default_font;
  _initialized = true;
  clear();
  return true;
}

//---------------------------------------------------------------------------------------------------------------------
void context2d::done()
{
  if (!_initialized)
    return;

  if (!default_font->unref_check()) default_font = nullptr;
  _initialized = false;
}

//---------------------------------------------------------------------------------------------------------------------
void context2d::print_substring(float &x, float &y, const vec4 &color, const char *text, size_t length)
{
  auto f = _state.font;
  if (!f) return;

  if (_drawCalls.back().triangles)
    _drawCalls.back().length += length * 6;
  else
    _drawCalls.emplace_back(true, length * 6);

  size_t skippedChars = 0;
  auto *v = _geometry->alloc_vertices(length * 6);
  uint64_t prevID = 0;

  while (length)
  {
    auto &chi = f->char_infos[*text];
    if (chi.id > 0)
    {
      int ker = f->kernings[prevID | (static_cast<uint64_t>(chi.id) << 32ull)];
      float ox = x + static_cast<float>(chi.offset.x + ker);
      float oy = y + static_cast<float>(chi.offset.y);

      v->pos = vec2(ox, oy);
      v->color = color;
      v->uv = chi.uv[0];
      ++v; // 1
      v->pos = vec2(ox + chi.size.x, oy);
      v->color = color;
      v->uv = chi.uv[1];
      ++v; // 2
      v->pos = vec2(ox + chi.size.x, oy + chi.size.y);
      v->color = color;
      v->uv = chi.uv[2];
      ++v; // 3
      v[0] = v[-1];
      ++v; // 4
      v->pos = vec2(ox, oy + chi.size.y);
      v->color = color;
      v->uv = chi.uv[3];
      ++v; // 5
      v[0] = v[-5];
      ++v;

      x += static_cast<float>(chi.x_advance + ker);
    }
    else
      ++skippedChars;

    prevID = static_cast<uint64_t>(chi.id);
    ++text;
    --length;
  }

  _geometry->pop_vertices(skippedChars * 6);
  _drawCalls.back().length -= skippedChars * 6;
}


}
#endif // __GL3D_2D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
