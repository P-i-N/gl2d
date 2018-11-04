#ifndef __GL2D_H__
#define __GL2D_H__

#include "gl3d.h"

namespace gl3d {

namespace detail {

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static const char *vertex_shader_code2d = R"GLSHADER(
GL3D_VERTEX_POS(vec2);
GL3D_VERTEX_COLOR(vec4);
GL3D_VERTEX_UV0(vec2);

uniform vec2 u_ScreenSize;

out vec4 Color;
out vec2 UV;

void main()
{
	vec2 clipPos = ((vertex_pos + vec2(0.375)) / u_ScreenSize);
	clipPos.y = 1.0 - clipPos.y;
	clipPos = clipPos * 2.0 - 1.0;

	gl_Position = vec4(clipPos, 0, 1);
	Color = vertex_color;
	UV = vertex_uv0;
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

//---------------------------------------------------------------------------------------------------------------------
static bool is_base64(uint8_t c) { return (isalnum(c) || (c == '+') || (c == '/')); }

//---------------------------------------------------------------------------------------------------------------------
std::vector<uint8_t> base64_decode(const char *encoded_string, size_t decompressedLength = 0)
{
	static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	const char *data = encoded_string;
	size_t len = strlen(encoded_string);

	if (len < decompressedLength)
	{
		char *unrolled = new char[decompressedLength + 1];
		unrolled[decompressedLength] = 0;

		for (size_t i = 0, j = 0; i < len; ++i)
		{
			if (encoded_string[i] != '~')
				unrolled[j++] = encoded_string[i];
			else
				for (size_t k = 0, S = 3 + base64_chars.find(encoded_string[++i]); k < S; ++k)
					unrolled[j++] = encoded_string[i - 2];
		}

		data = unrolled;
		len = decompressedLength;
	}

	int i = 0, cursor = 0;
	uint8_t char_array_4[4], char_array_3[3];
	std::vector<uint8_t> result;

	while (len-- && (data[cursor] != '=') && is_base64(data[cursor]))
	{
		char_array_4[i++] = data[cursor++];

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
		for (auto &j : char_array_4) j = static_cast<uint8_t>(base64_chars.find(j));

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (int j = 0; (j < i - 1); j++) result.push_back(char_array_3[j]);
	}

	if (data != encoded_string)
		delete [] data;

	return result;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct font
{
	using ptr = std::shared_ptr<font>;

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
	texture::ptr font_texture = std::make_shared<texture>();

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
		font_texture->set_wrap(gl_api.CLAMP_TO_EDGE);

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
	font(const char *base64Data, size_t length = 0): font(base64_decode(base64Data, length)) { }
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct vertex2d : layout<vertex_pos<vec2>, vertex_color<vec4>, vertex_uv0<vec2>, vertex_uv1<ivec4>>
{

};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

struct draw_call
{
	bool triangles;     // true for triangles, false for lines
	size_t start_index; // first vertex index
	size_t length;      // number of vertices

	draw_call(bool tris, size_t start, size_t len): triangles(tris), start_index(start), length(len) { }
	size_t end() const { return start_index + length; }
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
		_geometry->clear();
		_drawCalls.clear();
		_layers.clear();
		new_layer();
		_state.color = vec4::one();
	}

	size_t new_layer()
	{
		_layers.push_back(_drawCalls.size());

		if (_drawCalls.empty())
			_drawCalls.emplace_back(true, 0, 0);
		else
			_drawCalls.emplace_back(true, _drawCalls.back().end(), 0);

		return _layers.size() - 1;
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
			_drawCalls.emplace_back(false, _drawCalls.back().end(), 2);
		
		auto *v = _geometry->alloc_vertices(2);
		v->pos = a;
		v->color = _state.color;
		v->uv0 = vec2(1, 1);
		++v;
		v->pos = b;
		v->color = _state.color;
		v->uv0 = vec2(1, 1);
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
				_drawCalls.emplace_back(true, _drawCalls.back().end(), 6);
			
			auto *v = _geometry->alloc_vertices(6);
			v->pos = a;
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
			++v;
			v->pos = vec2(b.x, a.y);
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
			++v;
			v->pos = vec2(a.x, b.y);
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
			
			v[1] = *v;
			v[2] = v[-1];
			v += 3;

			v->pos = vec2(b.x, b.y);
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
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
	
	void triangle(const vec2 &a, const vec2 &b, const vec2 &c, bool filled = false)
	{
		if (filled)
		{
			if (_drawCalls.back().triangles)
				_drawCalls.back().length += 3;
			else
				_drawCalls.emplace_back(true, _drawCalls.back().end(), 3);

			auto *v = _geometry->alloc_vertices(3);
			v->pos = a;
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
			++v;
			v->pos = b;
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
			++v;
			v->pos = c;
			v->color = _state.color;
			v->uv0 = vec2(1, 1);
		}
		else
		{
			line(a, b);
			line(b, c);
			line(c, a);
		}
	}

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

	void begin_render(context3d &ctx, int x, int y, int width, int height)
	{
		using namespace detail;

		ctx.clear();

		glViewport(x, y, width, height);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		ctx.bind_geometry(_geometry);
		ctx.bind_program(_program);
		ctx.set_uniform("u_ScreenSize", vec2(width, height));
		ctx.set_uniform("u_FontTexture", _state.font->font_texture);
	}

	void render_layer(size_t index)
	{
		size_t start_dc = _layers[index];
		size_t end_dc = (index < _layers.size() - 1) ? _layers[index + 1] : _drawCalls.size();

		for (size_t i = start_dc; i < end_dc; ++i)
		{
			auto &dc = _drawCalls[i];
			glDrawArrays(dc.triangles ? GL_TRIANGLES : GL_LINES, static_cast<GLint>(dc.start_index), static_cast<GLsizei>(dc.length));
		}
	}

	void end_render()
	{
		clear();
	}

	void render(context3d &ctx, int x, int y, int width, int height)
	{
		begin_render(ctx, x, y, width, height);

		for (size_t i = 0, S = _layers.size(); i < S; ++i)
			render_layer(i);

		end_render();
	}

	void render(context3d &ctx, int width, int height) { render(ctx, 0, 0, width, height); }

private:
	void print_substring(float &x, float &y, const vec4 &color, const char *text, size_t length);

	bool _initialized = false;
	detail::state _state;
	program::ptr _program = std::make_shared<program>();
	geometry<detail::vertex2d>::ptr _geometry = std::make_shared<geometry<detail::vertex2d>>();
	std::vector<detail::draw_call> _drawCalls;
	std::vector<size_t> _layers;
};

extern detail::font::ptr default_font;
extern detail::font::ptr monospace_font;

}

#endif // __GL2D_H__

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#if defined(GL3D_IMPLEMENTATION)
#ifndef __GL3D_2D_H_IMPL__
#define __GL3D_2D_H_IMPL__
namespace gl3d {

detail::font::ptr default_font;
detail::font::ptr monospace_font; // TODO

//---------------------------------------------------------------------------------------------------------------------
static const size_t default_base64_length = 5484;
static const char *default_base64 =
"gABAAAgHXACxN978GB9gAwMDbn8IDEYAGWbGhoEgYAMDA2NgPgyGgBhmxoZdXmADAwNjMEsMhoAMxsaGWVt+OzuzZzALDIaADMbGhllbY2dnG2MYHrgDQQzG"
"xoZZW2NjYw9jGDwMBkEMxsaGWVtjY2MHYwxoDAYiDMbGhlk2Y2NjD2MMaQwGIgzGxvyYAHNjYxtjDD4MBiIYZsZgGB9uP2MzYwAIDAYUGGbGwA0A~EwCAccF"
"LA33gAAMAz74ePjMwA~GP3gMGzM2NjQOwogNDIkHP2N4DBs2NjY0G8GcHQ7JjGFjzAwbNjY2MJvAnD3P34xhY8z8G+YzNjCbwJT1zZ+HYT/MDBs25jcATo73"
"5IyEs2Eb/g0bNgY2NCCb90Tsz7ZhM4YNGzMGMzQgm/cE7M+cYWOGDfvh4+EDEBtjBEzCnGHDA~GgOYwRMgjc/AAA+Pj5jO34A~GH4+Y2FhM2djPz88PkNjY2"
"NjQ2NgYBtjY2NjBkNHY2NjY0NjYGAPY2NjYwNDRzZjY2MDYzA8B2NjY2M/A00cNmNjPmMYYA9jc2M/Y3NZHDZjc2BjDGAbP24/Y2NjWRw2Y25hYwZhMwNgA2"
"NjY3E2HGNgYT5/PmMDYANjY2NxYxxjPj4A~EDPz5+YWMcPgAAYLxtALAdA~GB+n2cYjAGMMzB+nsFv9psxGAP2mQEGgzE4BrHBYAabPRgDZpgB2IAxNAawwW"
"AGmzEYA2aYAdiAMTI+mMHAg/EwGB9mmA0GgzExYIzBz8PwMBgDZpiNAYwxf2CMwYBhYDAYA2aYbQAwADBggMGAMWAwGAPGvQUA~AwYYzBgDEwMBjDAwAAcAg"
"DMD6M34/xM/wAAMC3xviIBA~HmDke9taKTwgz+47PGAJn5plNAzPgCScQM5vZ2BiCbDbzDIMx0AIAALaZ2dgYAmw2YzjDMMA~AC2mdnY2J/P8/NgwzAGA~A/"
"pnZ2BjCzDOYZcM3BgAAAMyY2dgcwowxmjkeA~CDMmJmPG4KP4QEA~aH/bNvv3A~M22YDA~NNsGA~NH8BA~/A~/A~/A~GPDF8A~AgA~AKQAzAAMB/w4EIQAAA"
"HwACgACCQEDBSIA~ALADMABQMAAgYjA~AJQANAAgJAAMJJA~GHDAACCCUA~D0ADQkAAw4mA~ALgANAAgJAAMJJwAAABEAMwACAwACAygA~AiA~ABAwAAgUpA"
"~ALA~BQMAAIFKgAAAGQAKQAGBQECCCsA~AmACsABwcBBAosA~ACAAzAAIEAQoELQAAACQAMwAEAQAHBS4A~AYADMAAgIBCgQvA~AHA~BUMAAIHMA~AFAAFQA"
"HCQADCDEA~A6ACEABgkBAwgyA~AWAAVAAcJAAMIMwAAAGAAFQAHCQADCDQA~CCEABwkAAwg1A~ACAAhAAcJAAMINgAAABAAFwAHCQADCDcAAAB4A~ABwkAAw"
"g4A~AZAALAAcJAAMIOQAAAGwACwAHCQADCDoAAABhACkAAgcBBQQ7A~AYgAfAAIJAQUEPA~AG4AHwAICAEECj0A~CDMABwQBBgo+A~AZQAfAAgIAQQKPwAAA"
"BAAIQAGCQADB0A~ABGA~ACQoAAwpBA~ASQALAAgJAAMJQgA~AgAFwAHCQADCEMAAAB0AAsABwkAAwhEA~AWwALAAgJAAMJRQAAAB4AIQAGCQADB0YAAABIAC"
"AABQkAAwZHA~AGAAXAAcJAAMISA~AFIACwAICQADCUkAAABaAB8ABAkAAwVKA~ATgAgAAUJAAMGSwAAAGgAFQAHCQADCEwA~AXACEABgkAAwdNA~AGgANAAo"
"JAAMLTgAAACAAFwAHCQADCE8A~A3AA0ACAkAAwlQA~DXAAcJAAMIUQAAADkA~AICwADCVIAAABAAAwACAkAAwlTA~ASAAWAAcJAAMIVA~AEEAIAAGCQADB1U"
"A~A4ABcABwkAAwhWA~AMAAXAAcJAAMIVwA~A4ADQALCQADDFgA~AoABcABwkAAwhZA~AJQAhAAYJAAMHWgAAACwAIQAGCQADB1sA~AxA~ABAwAAgVcA~AFgA"
"~AUMAAIHXQAAACcA~AEDAACBV4AAAB1ACgACAQBAwpfA~AGwAzAAgBAA0IYA~ABQAMwADAgICB2EA~AuACsABgcABQdiA~AWA~BcKAAIIYwAAAFAAKgAFBwA"
"FBmQAAABQA~ABwoAAghlA~APAArAAYHAAUHZgAAAG8A~AFCgACBWcAAABAABYABwkABQhoA~AYA~BcKAAIIaQAAAF8AHwACCQADA2oAAABCA~BwsAAwRrA~A"
"aA~BYKAAIHbA~AHUA~ACCgACA20A~ALACsACgcABQtuA~AdwAfAAcHAAUIbwAAABYAKwAHBwAFCHA~ABwABUABwkABQhxA~AeAAVAAcJAAUIcgAAAFwAKQAE"
"BwAFBXMAAABKACoABQcABQZ0A~AVAAfAAUJAAMGdQAAAB4AKwAHBwAFCHYA~A1ACsABgcABQd3A~DrAAoHAAULeA~AEMAKgAGBwAFB3kA~AzACEABgkABQd6"
"A~AVgApAAUHAAUGewA~A8A~AGDAACB3wA~A2A~BgwCAgd9A~ACA~BYMAAIHfgAAAGsAKAAJBAAGCgA~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A"
"~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~p=";

//---------------------------------------------------------------------------------------------------------------------
static const size_t monospace_base64_length = 5484;
static const char *monospace_base64 =
"gABAAONB4A4YeuHxDCAOR34i32czQTAYGGFjmAgwGyUzttlsE/IwGIBhZowZMBs1M7bZbPNaMBjfYGyMERAbFx+U2WybWjAY2GBsjDEYDggDnNls23occNhg"
"bIwxCD88PwzfZ9viMRjYYGzcIIw5LGMM2ADbojEY2GBseGCMMSpjDNhgu6ExGNhgbGBABH87PgfYYAv4MBiYYWZgwAYA~ETIDAYGWNiwAEAIIgHh+dj8yDgD"
"g564QGA5GzMhMdmZgMA~DCAh+RsbIxmbgyAAWxwcA8zzOjvTWTMZm8MgwFswBkMY2aAZM1mbOZtbIMBbIAZDGBmQGLNYuzvbGyDj++DGczDY+5vj8MGxmYGs"
"9ls9jwMg2FMYo7DA8bjA5vZbDYYDIPBTGIOA~CCP2WwGGAyDgQ8AAPh9ZjwPn9lsNhgMgwHA5+dgzGaGGZuZ7zMY3wcAzwyiYMxmgxkzA~EjMEMo2HMZoMZg"
"M9n5pkxOIzBB6FhfGYDH7MBbBbbOAy/x4wwYQxmAxieAWwGWzQGDN7M8GMMZgYMno/nB3swPgzYTBBjDDw8BwwYbIZ5MGYM2OcXAgA~BcGGzGWDBmjA8AAAC"
"fmWdmGhhsZtgwZjgA2L+gsdnMY7PHZ/ab/TwADMy2oLHZj0EzA~EnhnGtqSx2UBBgN8n+r5h+4wxw7a7sdlBMx7YLBqwYRsMYMY2Gx+fbwYzzKwasGEbjDHM"
"NhsA~BM8ys+rBhG4wZGAAA/h4AAB7GYxuwofoMDQDfzgA~B+wmYbsKEaDAFswNwA~BM8NmG7IhGt44bd8A~CDPBbPucLxoAdG0A~EeA~EQOQA~GM/u40Ufg6"
"EA~GB7YbTZsyI+xAQA~FD32A2bAzjtwEA~Fj9lgNijGjwEA~GLzZYDYoAoMBA~Hw32DmOR8A~JHwA~/A~/A~/A~/A~/A~WPDF8A~AgA~AbgAjAAMB/w4HIQA"
"AAH0A~ACCQMDByIAAABKACUABQMBAwcjA~ARQALAAcIAAQHJA~BsA~AGDAECByUAAABYA~ABwkAAwcmA~ATwA~AgJAAMHJwAAAFYAJAACAwMDBygA~AmA~AB"
"AwCAwcpA~AMA~BQMAQMHKgAAAC4AKQAGBQEDBysA~A1ACkABgUBBgcsA~AfAAcAAMFAQkHLQAAAGkAIwAEAQIIBy4AAABeACQAAgICCgcvA~ASQA~AUKAAMH"
"MA~AG0ACgAHCAAEBzEA~AqABcABggBBAcyA~AHAAXAAYIAQQHMwA~A4AFwAGCAEEBzQAAABlAAoABwgABAc1A~ABwAXAAYIAQQHNgAAADEAFwAGCAEEBzcA~"
"AHACAABggBBAc4A~DhAAYIAQQHOQAAAHcAEwAGCAAEBzoAAAB9AAoAAgYCBgc7A~APgAfAAMIAgYHPA~AEgAHQAFBwEFBz0AAABQACQABQMBBwc+A~AQgAeA"
"AUHAQUHPwAAABwADQAFCQIDB0A~BDA~ABwwAAwdBA~AVAAUAAYIAQQHQgAAAEYAFAAGCAEEB0MAAABwABMABggBBAdEA~AdQAKAAcIAQQHRQAAABsAIAAFCA"
"EEB0YA~AzACAABQgBBAdHA~APQAMAAcIAAQHSA~ABUAFwAGCAEEB0kA~A5AB8ABAgCBAdKA~AIQAgAAUIAQQHSwAAACMAFwAGCAEEB0wA~AnACAABQgBBAdN"
"A~ALQAgAAUIAQQHTgAAAE0ACwAHCAEEB08AAABdAAoABwgABAdQA~AYgATAAYIAQQHUQAAADoA~AHCwAEB1IA~AOACAABggBBAdTA~APwAVAAYIAQQHVA~AF"
"sAEwAGCAEEB1UAAABpABMABggBBAdWA~AVQAKAAcIAAQHVwAAABUAIAAFCAEEB1gA~CBgABggBBAdZA~ANAANAAgIAAQHWgAAAE0AFAAGCAEEB1sA~A1A~AB"
"AwBAwdcA~AQgA~AYKAQMHXQAAACsA~AEDAIDB14A~A8ACgABQQBBAdfA~AYQAjAAcBAA4HYA~AFkAJAAEAgEDB2EA~AHACkABgYBBgdiA~AFQANAAYJAQMHY"
"wAAABwAKQAFBgEGB2QA~AOAA0ABgkAAwdlA~AbgAcAAYGAQYHZgAAACIADQAFCQEDB2cAAABgA~ABwkABgdoA~ABwANAAYJAQMHaQAAAC4ADQAFCQEDB2oA~"
"AgA~ABQwBAwdrA~DOAAYJAQMHbA~ACgADQAFCQEDB20AAABOAB0ACAYABgduA~AFQApAAYGAQYHbwAAAF8AHAAHBgAGB3A~AB2A~ABgkBBgdxA~AbwA~AYJA"
"AYHcgA~A4AKQAGBgEGB3MA~CCoABgYBBgd0A~AOAAWAAYIAAQHdQAAAGcAHAAGBgEGB3YA~AiACkABQYBBgd3A~AVwAdAAcGAAYHeA~AHUAHAAGBgEGB3kAA"
"ABoA~ABgkABgd6A~AKAApAAUGAQYHewAAABIA~AGDAADB3wA~Gg0DAgd9A~AGQA~AYMAQMHfgAAAEIAJgAHAwAHBwA~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A"
"~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~/A~p=";

//---------------------------------------------------------------------------------------------------------------------
bool context2d::init()
{
	using namespace detail;

	if (_initialized)
		return true;

	gl_api.init();

	_program->set_vert_source(vertex_shader_code2d);
	_program->set_frag_source(fragment_shader_code2d);

	if (!default_font)
		default_font = std::make_shared<detail::font>(default_base64, default_base64_length);

	if (!monospace_font)
		monospace_font = std::make_shared<detail::font>(default_base64, default_base64_length);
	
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

	default_font = nullptr;
	monospace_font = nullptr;
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
		_drawCalls.emplace_back(true, _drawCalls.back().end(), length * 6);

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
			v->uv0 = chi.uv[0];
			++v; // 1
			v->pos = vec2(ox + chi.size.x, oy);
			v->color = color;
			v->uv0 = chi.uv[1];
			++v; // 2
			v->pos = vec2(ox + chi.size.x, oy + chi.size.y);
			v->color = color;
			v->uv0 = chi.uv[2];
			++v; // 3
			v[0] = v[-1];
			++v; // 4
			v->pos = vec2(ox, oy + chi.size.y);
			v->color = color;
			v->uv0 = chi.uv[3];
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
