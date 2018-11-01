#ifndef __GL3D_BASE_H_IMPL__
	#define __GL3D_BASE_H_IMPL__
#endif

#include "gl3d_base.h"

#include <cstdarg>

#define GL3D_FORMAT_LOG_TEXT(_Input) \
	if (!(_Input)) return; \
	va_list ap; \
	va_start(ap, _Input); \
	vsnprintf(detail::tl_logBuffer, detail::logBufferSize, _Input, ap); \
	va_end(ap)

namespace gl3d {

decltype( on_log_message ) on_log_message;

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
std::vector<char> load_all_chars( std::istream &is, bool addNullTerm, size_t size )
{
	if ( !size )
	{
		is.seekg( 0, std::ios_base::end );
		size = is.tellg();
		is.seekg( 0, std::ios_base::beg );
	}

	std::vector<char> result( size );
	is.read( result.data(), size );
	if ( addNullTerm ) result.push_back( 0 );

	return std::move( result );
}

}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------------------------------------
void log::info( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message_type::info, detail::tl_logBuffer );
}

//---------------------------------------------------------------------------------------------------------------------
void log::success( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message_type::success, detail::tl_logBuffer );
}

//---------------------------------------------------------------------------------------------------------------------
void log::warning( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message_type::warning, detail::tl_logBuffer );
}

//---------------------------------------------------------------------------------------------------------------------
void log::error( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message_type::error, detail::tl_logBuffer );
}

//---------------------------------------------------------------------------------------------------------------------
void log::fatal( const char *fmt, ... )
{
	GL3D_FORMAT_LOG_TEXT( fmt );
	on_log_message.call( log::message_type::fatal, detail::tl_logBuffer );
}

} // namespace gl3d

#undef GL3D_FORMAT_LOG_TEXT
