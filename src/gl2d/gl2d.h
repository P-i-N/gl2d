#ifndef __GL2D_H__
#define __GL2D_H__

#include "gl3d.h"

namespace gl3d {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct mat3
{
  float m[9];

  mat3() { m[0] = m[1] = m[2] = m[3] = m[4] = m[5] = m[6] = m[7] = m[8] = 0.0f; }

  template <typename T>
  mat3(T diag)
  {
    m[0] = m[4] = m[8] = static_cast<float>(diag);
    m[1] = m[2] = m[3] = m[5] = m[6] = m[7] = 0.0f;
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct rect
{
  vec2 min, max;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace detail {
  
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *vertex_shader_code = R"GLSHADER(
#version 330

layout(location = 0) in vec2 vert_Position;
layout(location = 1) in vec4 vert_Color;
layout(location = 2) in vec2 vert_UV;

uniform vec2 u_ScreenSize;

out vec4 Color;
out vec2 UV;

void main()
{
  vec2 clipPos = (vert_Position / u_ScreenSize);
  clipPos.y = 1.0 - clipPos.y;
  clipPos = clipPos * 2.0 - 1.0;

  gl_Position = vec4(clipPos, 0, 1);
  Color = vert_Color;
  UV = vert_UV;
}
)GLSHADER";

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *fragment_shader_code = R"GLSHADER(
#version 330

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

//---------------------------------------------------------------------------------------------------------------------
static const int font_width = 288;
static const int font_height = 42;
static const int font_char_width = 9;
static const int font_char_height = 14;
static const int font_char_step = 8;
static const int font_line_height = 12;

//---------------------------------------------------------------------------------------------------------------------
static const char *font_base64 =
  "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";

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

struct vertex2d : layout<vec2, rgba_color, vec2>
{
  vec2 pos;
  rgba_color color;
  vec2 uv;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct draw_call
{
  bool triangles; // true for triangles, false for lines
  size_t length;  // number of vertices

  draw_call(bool tris, size_t len)
    : triangles(tris)
    , length(len)
  {
    
  }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct state
{
  rect viewport;
  rect scissors;
  gl3d::rgba_color color;
};

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class context2d
{
public:
  context2d()
  {
    clear();
  }

  virtual ~context2d()
  {
    done();
  }
  
  bool initialized() const { return _initialized; }

  bool init()
  {
    using namespace detail;

    if (_initialized)
      return true;

    _initialized = true;

    gl.init();
    gl.GenBuffers(1, &_vbo);
    gl.GenVertexArrays(1, &_vao);

    // Initialize VBO and VAO
    {
      gl.BindBuffer(gl_api::ARRAY_BUFFER, _vbo);
      gl.BindVertexArray(_vao);
      vertex2d::init_vao();
      gl.BindVertexArray(0);
      gl.BindBuffer(gl_api::ARRAY_BUFFER, 0);
    }

    _technique->set_vert_source(vertex_shader_code);
    _technique->set_frag_source(fragment_shader_code);

    if (!_vertShader.compile(gl.VERTEX_SHADER, vertex_shader_code)) { done(); return false; }
    if (!_fragShader.compile(gl.FRAGMENT_SHADER, fragment_shader_code)) { done(); return false; }
    if (!_program.link({ _vertShader, _fragShader })) { done(); return false; }

    _uScreenSize = gl.GetUniformLocation(_program, "u_ScreenSize");
    _uFontTexture = gl.GetUniformLocation(_program, "u_FontTexture");

    // Initialize font texture
    {
      auto fontImage = base64_decode(detail::font_base64);

      uint32_t *image = new uint32_t[detail::font_width * detail::font_height];
      memset(image, 0, detail::font_width * detail::font_height * sizeof(uint32_t));
      
      uint32_t *cursorOutput = image;
      const uint8_t *cursorInput = fontImage.data();

      for (int y = 0; y < detail::font_height; ++y)
      {
        for (int x = 0; x < detail::font_width; x += 8, ++cursorInput)
        {
          auto byte = *cursorInput;

          for (int i = 0; i < 8; ++i, ++cursorOutput, byte >>= 1)
          {
            if (byte & 1)
            {
              *cursorOutput = 0xFFFFFFFFu;
              *(cursorOutput + detail::font_width + 1) = 0xFF000000u;
            }
          }
        }
      }

      image[detail::font_width * detail::font_height - 1] = 0xFFFFFFFFu;

      glGenTextures(1, &_fontTexture);
      glBindTexture(GL_TEXTURE_2D, _fontTexture);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, detail::font_width, detail::font_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, gl_api::CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, gl_api::CLAMP_TO_EDGE);
      glBindTexture(GL_TEXTURE_2D, 0);

      delete [] image;
    }
    
    return true;
  }

  void done()
  {
    if (!_initialized)
      return;

    if (_vbo)
    {
      gl.DeleteBuffers(1, &_vbo);
      _vbo = 0;
    }

    if (_vao)
    {
      gl.DeleteVertexArrays(1, &_vao);
      _vao = 0;
    }

    _vertShader.destroy();
    _fragShader.destroy();
    _program.destroy();

    if (_fontTexture)
    {
      glDeleteTextures(1, &_fontTexture);
      _fontTexture = 0;
    }

    _initialized = false;
  }

  void clear()
  {
    _drawCalls.clear();
    _drawCalls.emplace_back(true, 0);
    _vertexCursor = 0;
    _state.color = gl3d::rgba_color(1, 1, 1, 1);
  }

  void color(const rgba_color &c) { _state.color = c; }

  void color(float r, float g, float b, float a = 1.0f) { _state.color = rgba_color(r, g, b, a); }

  void color(uint32_t argb) { _state.color = rgba_color(argb); }

  const rgba_color &color() const { return _state.color; }

  void line(const vec2 &a, const vec2 &b)
  {
    if (!_drawCalls.back().triangles)
      _drawCalls.back().length += 2;
    else
      _drawCalls.emplace_back(false, 2);
    
    auto *v = alloc_vertices(2);
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
      
      auto *v = alloc_vertices(6);
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
      line(a.x, a.y, b.x, a.y);
      line(b.x, a.y, b.x, b.y);
      line(b.x, b.y, a.x, b.y);
      line(a.x, b.y, a.x, a.y);
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
    gl3d::rgba_color color = _state.color;

    while (text < end)
    {
      const char *mark = strchr(text, '^');
    
      if (!mark)
        mark = end;

      int partLen = mark - text;

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
            case '0': color = gl3d::rgba_color(0, 0, 0, 1); break;
            case '1': color = gl3d::rgba_color(0, 0, h, 1); break;
            case '2': color = gl3d::rgba_color(0, h, 0, 1); break;
            case '3': color = gl3d::rgba_color(0, h, h, 1); break;
            case '4': color = gl3d::rgba_color(h, 0, 0, 1); break;
            case '5': color = gl3d::rgba_color(h, 0, h, 1); break;
            case '6': color = gl3d::rgba_color(h, h, 0, 1); break;
            case '7': color = gl3d::rgba_color(h, h, h, 1); break;
            case '8': color = gl3d::rgba_color(q, q, q, 1); break;
            case '9': color = gl3d::rgba_color(q, h, 1, 1); break;
            case 'a': color = gl3d::rgba_color(q, 1, q, 1); break;
            case 'b': color = gl3d::rgba_color(q, 1, 1, 1); break;
            case 'c': color = gl3d::rgba_color(1, h, q, 1); break;
            case 'd': color = gl3d::rgba_color(1, q, 1, 1); break;
            case 'e': color = gl3d::rgba_color(1, 1, 0, 1); break;

            default:
            case 'f': color = gl3d::rgba_color(1, 1, 1, 1); break;
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

    glViewport(x, y, width, height);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Copy vertices into VBO
    gl.BindBuffer(gl_api::ARRAY_BUFFER, _vbo);
    gl.BufferData(gl_api::ARRAY_BUFFER, sizeof(vertex2d) * _vertexCursor, _vertices.data(), gl_api::STREAM_DRAW);
    gl.BindVertexArray(_vao);
    gl.UseProgram(_program);

    gl.ActiveTexture(gl_api::TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, _fontTexture);

    vec2 screenSize(width, height);
    gl.Uniform2fv(_uScreenSize, 1, screenSize.data());
    gl.Uniform1i(_uFontTexture, 0);

    size_t startVertex = 0;
    for (auto &&dc : _drawCalls)
    {
      if (!dc.length) continue;

      glDrawArrays(dc.triangles ? GL_TRIANGLES : GL_LINES, static_cast<GLint>(startVertex), static_cast<GLsizei>(dc.length));
      startVertex += dc.length;
    }

    gl.BindBuffer(gl_api::ARRAY_BUFFER, 0);
    gl.BindVertexArray(0);
    gl.UseProgram(0);

    clear(); 
  }

  void render(int width, int height) { render(0, 0, width, height); }

private:
  detail::vertex2d *alloc_vertices(size_t count)
  {
    if (_vertexCursor + count > _vertices.size())
      _vertices.resize(_vertexCursor + count);

    auto result = _vertices.data() + _vertexCursor;
    _vertexCursor += count;

    return result;
  }

  void print_substring(float &x, float &y, const gl3d::rgba_color &color, const char *text, size_t length)
  {
    if (_drawCalls.back().triangles)
      _drawCalls.back().length += length * 6;
    else
      _drawCalls.emplace_back(true, length * 6);

    size_t skippedChars = 0;
    auto *v = alloc_vertices(length * 6);

    const float uvW = static_cast<float>(detail::font_char_width) / detail::font_width;
    const float uvH = static_cast<float>(detail::font_char_height) / detail::font_height;

    while (length)
    {
      char ch = *text;
      if (ch > 32)
      {
        ch -= 32;
        float uvX = (ch % (detail::font_width / detail::font_char_width)) * uvW;
        float uvY = (ch / (detail::font_width / detail::font_char_width)) * uvH;

        v->pos = vec2(x, y);
        v->color = color;
        v->uv = vec2(uvX, uvY);
        ++v;
        v->pos = vec2(x + detail::font_char_width, y);
        v->color = color;
        v->uv = vec2(uvX + uvW, uvY);
        ++v;
        v->pos = vec2(x, y + detail::font_char_height);
        v->color = color;
        v->uv = vec2(uvX, uvY + uvH);

        v[1] = *v;
        v[2] = v[-1];
        v += 3;

        v->pos = vec2(x + detail::font_char_width, y + detail::font_char_height);
        v->color = color;
        v->uv = vec2(uvX + uvW, uvY + uvH);
        ++v;
      }
      else
        ++skippedChars;

      x += detail::font_char_step;
      ++text;
      --length;
    }

    _vertexCursor -= skippedChars * 6;
    _drawCalls.back().length -= skippedChars * 6;
  }

  bool _initialized = false;

  detail::state _state;


  GLuint _vbo = 0;
  
  GLuint _vao = 0;

  detail::gl_resource_shader _vertShader;

  detail::gl_resource_shader _fragShader;

  detail::gl_resource_program _program;

  detail::ptr<technique> _technique = new technique();

  GLint _uScreenSize = -1;

  GLint _uFontTexture = -1;

  GLuint _fontTexture = 0;

  std::vector<detail::vertex2d> _vertices;

  size_t _vertexCursor = 0;

  std::vector<detail::draw_call> _drawCalls;
};

// Global application instance
extern context2d *current_context2d;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
void color(const rgba_color &c)
{
  if (current_context2d)
    current_context2d->color(c);
}

//---------------------------------------------------------------------------------------------------------------------
void color(float r, float g, float b, float a = 1.0f)
{
  if (current_context2d)
    current_context2d->color(r, g, b, a);
}

//---------------------------------------------------------------------------------------------------------------------
void color(uint32_t argb)
{
  if (current_context2d)
    current_context2d->color(argb);
}

//---------------------------------------------------------------------------------------------------------------------
void line(const vec2 &a, const vec2 &b)
{
  if (current_context2d)
    current_context2d->line(a, b);
}

//---------------------------------------------------------------------------------------------------------------------
void line(float x1, float y1, float x2, float y2)
{
  if (current_context2d)
    current_context2d->line(x1, y1, x2, y2);
}

//---------------------------------------------------------------------------------------------------------------------
void linei(int x1, int y1, int x2, int y2)
{
  if (current_context2d)
    current_context2d->linei(x1, y1, x2, y2);
}

//---------------------------------------------------------------------------------------------------------------------
void rectangle(const vec2 &a, const vec2 &b, bool filled = false)
{
  if (current_context2d)
    current_context2d->rectangle(a, b, filled);
}

//---------------------------------------------------------------------------------------------------------------------
void rectangle(float x1, float y1, float x2, float y2, bool filled = false)
{
  if (current_context2d)
    current_context2d->rectangle(x1, y1, x2, y2, filled);
}

//---------------------------------------------------------------------------------------------------------------------
void rectanglei(int x1, int y1, int x2, int y2, bool filled = false)
{
  if (current_context2d)
    current_context2d->rectanglei(x1, y1, x2, y2, filled);
}

//---------------------------------------------------------------------------------------------------------------------
void text(const vec2 &pos, const char *fmt, va_list &ap)
{
  if (current_context2d)
    current_context2d->text(pos, fmt, ap);
}

//---------------------------------------------------------------------------------------------------------------------
void text(float x, float y, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  text(vec2(x, y), fmt, ap);
  va_end(ap);
}

//---------------------------------------------------------------------------------------------------------------------
void text(int x, int y, const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  text(vec2(x, y), fmt, ap);
  va_end(ap);
}

}

#endif // __GL2D_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef GL3D_IMPLEMENTATION
#ifndef __GL2D_H_IMPL__
#define __GL2D_H_IMPL__

namespace gl3d {
  
static context2d *current_context2d = nullptr;

}

#endif // __GL2D_H_IMPL__
#endif // GL3D_IMPLEMENTATION
