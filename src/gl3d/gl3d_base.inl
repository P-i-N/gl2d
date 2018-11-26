#ifndef __GL3D_BASE_H_IMPL__
	#define __GL3D_BASE_H_IMPL__
#endif

#include "gl3d_base.h"

#include <cstdarg>
#include <fstream>
#include <sstream>
#include <chrono>

#if defined(WIN32)
	#ifndef VC_EXTRALEAN
		#define VC_EXTRALEAN
	#endif

	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
#else
	#error Not implemented!
#endif

#define GL3D_FORMAT_LOG_TEXT(_Input) \
	if (!(_Input)) return; \
	uint64_t time = std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::steady_clock::now() - detail::g_logTimeOffset ).count(); \
	va_list ap; \
	va_start(ap, _Input); \
	vsnprintf(detail::tl_logBuffer, detail::logBufferSize, _Input, ap); \
	va_end(ap)

namespace gl3d {

detail::keyboard_state keyboard;
detail::mouse_state mouse;
detail::gamepad_state gamepad[detail::max_gamepads];
detail::space_navigator_state space_navigator;

decltype( on_tick ) on_tick;
decltype( on_input_event ) on_input_event;
decltype( on_log_message ) on_log_message;
decltype( on_vfs_load ) on_vfs_load;

namespace detail {

constexpr char *s_lineSeparator = "\n";
constexpr size_t logBufferSize = 1025;
thread_local char tl_logBuffer[logBufferSize];

//---------------------------------------------------------------------------------------------------------------------
std::string_view trim( std::string_view text )
{
	if ( text.empty() ) return text;

	size_t start = 0, end = text.length() - 1;
	while ( start < text.length() && isspace( text[start] ) ) ++start;
	while ( end > start && isspace( text[end] ) ) --end;

	return text.substr( start, end - start + 1 );
}

//---------------------------------------------------------------------------------------------------------------------
std::string_view to_string_view( bytes_t &bytes )
{
	return std::string_view( reinterpret_cast<const char *>( bytes.data() ), bytes.size() );
}

//---------------------------------------------------------------------------------------------------------------------
void for_each_line( std::string_view text, std::function<void( std::string_view, unsigned )> callback )
{
	size_t cursor = 0;
	unsigned lineNum = 0;
	while ( cursor <= text.length() )
	{
		auto sepPos = text.find( s_lineSeparator, cursor );
		if ( sepPos == std::string::npos )
			sepPos = text.length();

		callback( text.substr( cursor, sepPos - cursor ), ++lineNum );
		cursor = sepPos + strlen( s_lineSeparator );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void read_all_bytes( std::istream &is, bytes_t &bytes, bool addNullTerm, size_t size )
{
	if ( size == size_t( -1 ) )
	{
		is.seekg( 0, std::ios_base::end );
		size = is.tellg();
		is.seekg( 0, std::ios_base::beg );
	}

	bytes.resize( size );
	is.read( reinterpret_cast<char *>( bytes.data() ), size );
	if ( addNullTerm ) bytes.push_back( 0 );
}

//---------------------------------------------------------------------------------------------------------------------
bool unroll_includes( std::stringstream &ss, std::string_view sourceCode, const std::filesystem::path &cwd )
{
	bool result = true;

	for_each_line( sourceCode, [&]( std::string_view line, unsigned lineNum )
	{
		if ( !result ) return;

		bool addLine = true;

		if ( auto dir = trim( line ); !dir.empty() && dir[0] == '#' )
		{
			dir = trim( line.substr( 1 ) ); // Cut away '#' & trim
			if ( starts_with_nocase( dir, "include" ) )
			{
				addLine = false;
				dir = trim( dir.substr( 7 ) ); // Cut away "include" & trim

				bool isRelative = false;

				if ( dir[0] == '"' && dir.back() == '"' )
					isRelative = true;
				else if ( dir[0] == '<' && dir.back() == '>' )
					isRelative = false;
				else
				{
					log::error( "Invalid include directive at line %d", lineNum );
					result = false;
					return;
				}

				std::filesystem::path path = trim( dir.substr( 1, dir.length() - 2 ) );
				if ( isRelative )
					path = cwd / path;

				bytes_t bytes;
				if ( !vfs::load( path, bytes ) )
				{
					log::error( "Could not open file stream: %s", path.c_str() );
					result = false;
					return;
				}

				if ( !unroll_includes( ss, detail::to_string_view( bytes ), path.parent_path() ) )
					return;
			}
		}

		if ( addLine )
			ss << line << std::endl;
	} );

	return result;
}

//---------------------------------------------------------------------------------------------------------------------
void *get_proc_address( const char *name )
{
#if defined(WIN32)
	return wglGetProcAddress( name );
#else
#error Not implemented!
#endif
}

//---------------------------------------------------------------------------------------------------------------------
void keyboard_state::change_key_state( key k, bool down, unsigned id )
{
	bool old = ( *this )[k];
	if ( old != down )
	{
		key_down[+k] = down;
		input_event e( down ? input_event::type::key_down : input_event::type::key_up, id );
		e.keyboard.k = k;
		e.keyboard.ch = 0;
		on_input_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void mouse_state::change_button_state( mouse_button b, bool down, unsigned id )
{
	bool old = ( *this )[b];
	if ( old != down )
	{
		button_down[+b] = down;
		input_event e( down ? input_event::type::mouse_down : input_event::type::mouse_up, id );
		e.mouse.b = b;
		e.mouse.pos = pos;
		e.mouse.delta = { 0, 0 };
		on_input_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void mouse_state::change_position( ivec2 pos, unsigned id )
{
	if ( this->pos != pos )
	{
		input_event e( input_event::type::mouse_move, id );
		e.mouse.b = mouse_button::unknown;
		e.mouse.pos = pos;
		e.mouse.delta = pos - this->pos;
		this->pos = pos;
		on_input_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::change_button_state( gamepad_button b, bool down )
{
	bool old = ( *this )[b];
	if ( old != down )
	{
		button_down[+b] = down;
		input_event e( down ? input_event::type::gamepad_down : input_event::type::gamepad_up, UINT_MAX );
		e.gamepad.port = port;
		e.gamepad.b = b;
		on_input_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::change_axis_state( gamepad_axis axis, vec2 pos )
{
	auto oldPos = this->pos[+axis];
	if ( oldPos != pos )
	{
		this->pos[+axis] = pos;
		input_event e( input_event::type::gamepad_move, UINT_MAX );
		e.gamepad.port = port;
		e.gamepad.axis = axis;
		e.gamepad.pos = pos;
		e.gamepad.delta = pos - oldPos;
		on_input_event.call( e );
	}
}

//---------------------------------------------------------------------------------------------------------------------
unsigned gamepad_state::allocate_port()
{
	for ( unsigned i = 0; i < max_gamepads; ++i )
		if ( gamepad[i].port < 0 )
			return gamepad[i].port = i;

	return -1;
}

//---------------------------------------------------------------------------------------------------------------------
void gamepad_state::release_port( unsigned port )
{
	if ( port >= 0 && port < max_gamepads && gamepad[port].port == port )
		gamepad[port].port = -1;
}

//---------------------------------------------------------------------------------------------------------------------
struct mount_info
{
	std::filesystem::path path;
	std::function<bool( const std::filesystem::path &, std::vector<uint8_t> & )> callback;

	bool operator==( const std::filesystem::path &p ) const { return path == p; }
};

std::mutex g_mountInfosMutex;
std::vector<mount_info> g_mountInfos;

struct vfs_init { vfs_init() { vfs::mount( "." ); } } g_vfsInit;

std::chrono::steady_clock::time_point g_logTimeOffset = std::chrono::steady_clock::now();

struct log_init
{
	log_init()
	{
		using namespace std::chrono;

		on_log_message += [&]( const log::message & msg )
		{
			auto milis = static_cast<int>( msg.time % 1000ull );
			auto seconds = static_cast<int>( ( msg.time / 1000ull ) % 60 );
			auto minutes = static_cast<int>( msg.time / 60000ull );

			static std::recursive_mutex s_logMutex;
			std::scoped_lock lock( s_logMutex );

#if defined(WIN32)
			SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 8 );
			printf( "[%02d:%02d.%03d] ", minutes, seconds, milis );

			static int s_typeColors[] = { 7, 10, 14, 12, 14 | ( 4 << 4 ) };
			int color = s_typeColors[static_cast<size_t>( msg.type )];
			SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), color );
			printf( "%s\n", msg.text );
			SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7 );
#else
#error Not implemented!
#endif
		};
	}
} g_logInit;

} // namespace gl3d::detail

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
void log::info( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message { time, log::message_type::info, detail::tl_logBuffer } );
}

//---------------------------------------------------------------------------------------------------------------------
void log::success( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message{ time, log::message_type::success, detail::tl_logBuffer } );
}

//---------------------------------------------------------------------------------------------------------------------
void log::warning( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message{ time, log::message_type::warning, detail::tl_logBuffer } );
}

//---------------------------------------------------------------------------------------------------------------------
void log::error( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message{ time, log::message_type::error, detail::tl_logBuffer } );
}

//---------------------------------------------------------------------------------------------------------------------
void log::fatal( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message{ time, log::message_type::fatal, detail::tl_logBuffer } );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
void vfs::mount( const std::filesystem::path &path )
{
	auto absPath = std::filesystem::absolute( path );
	absPath.make_preferred();
	auto absPathStr = absPath.string();

	log::info( "Mounting VFS path: %s", absPathStr.c_str() );

	std::scoped_lock lock( detail::g_mountInfosMutex );
	auto iter = std::find( detail::g_mountInfos.begin(), detail::g_mountInfos.end(), absPath );
	if ( iter == detail::g_mountInfos.end() )
	{
		auto callback = [absPath, absPathStr]( const std::filesystem::path & relPath, detail::bytes_t &bytes )->bool
		{
			auto finalPath = std::filesystem::absolute( absPath / relPath );
			finalPath.make_preferred();
			auto finalPathStr = finalPath.string();

			if ( finalPathStr.length() <= absPathStr.length() )
				return false;

			if ( _memicmp( absPathStr.c_str(), finalPathStr.c_str(), absPathStr.length() ) )
				return false;

			if ( std::filesystem::is_regular_file( finalPath ) )
			{
				std::ifstream ifs( finalPath.c_str(), std::ios_base::in | std::ios_base::binary );
				if ( !ifs.is_open() )
				{
					log::error( "Could not open file: %s", finalPath.c_str() );
					return false;
				}

				size_t size = std::filesystem::file_size( finalPath );
				detail::read_all_bytes( ifs, bytes, false, size );
				return true;
			}

			return false;
		};

		on_vfs_load += callback;
		detail::g_mountInfos.push_back( { absPath, callback } );
	}
}

//---------------------------------------------------------------------------------------------------------------------
bool vfs::unmount( const std::filesystem::path &path )
{
	auto absPath = std::filesystem::absolute( path );

	std::scoped_lock lock( detail::g_mountInfosMutex );
	auto iter = std::find( detail::g_mountInfos.begin(), detail::g_mountInfos.end(), absPath );
	if ( iter == detail::g_mountInfos.end() )
		return false;

	detail::g_mountInfos.erase( iter );
	return true;
}

//---------------------------------------------------------------------------------------------------------------------
bool vfs::load( const std::filesystem::path &path, detail::bytes_t &bytes )
{
	return on_vfs_load.call( path, bytes );
}

} // namespace gl3d

#undef GL3D_FORMAT_LOG_TEXT
