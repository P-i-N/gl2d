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

constexpr size_t logBufferSize = 1025;
thread_local char tl_logBuffer[logBufferSize];

}

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
